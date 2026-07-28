#include <chrono>
#include <iostream>
#include <optional>
#include <string>
#include <thread>
#include <unordered_map>
#include <unordered_set>
#include <variant>
#include <vector>

#include "ConnectionManager.h"
#include "GameManager.h"
#include "GameStateSerializer.h"
#include "Matchmaker.h"
#include "SqliteUserRepository.h"
#include "WebSocketServer.h"
#include "WireFormat.h"
#include "net/CommandDispatcher.h"

namespace {

constexpr unsigned short kPort = 9001;
constexpr int kTickMs = 30;

// Ties every server-side piece built in this branch into the single
// sequential game-loop thread the plan specifies (§4.3): one WebSocketServer,
// one GameManager registry, one Matchmaker, one SQLite-backed
// UserRepository, all driven by one fixed-interval loop in run().
class Server {
public:
    Server() : connections_(server_), matchmaker_(games_), users_("kungfuchess.db") {
        server_.on_disconnect([this](int connection_id) { handle_disconnect(connection_id); });
        server_.on_message(
            [this](int connection_id, const std::string& message) { handle_message(connection_id, message); });
    }

    [[noreturn]] void run() {
        std::cout << "Kung-Fu-Chess server listening on port " << kPort << "\n";
        for (;;) {
            server_.poll();
            tick_rooms();
            std::this_thread::sleep_for(std::chrono::milliseconds(kTickMs));
        }
    }

private:
    WebSocketServer server_{ kPort };
    ConnectionManager connections_;
    GameManager games_;
    Matchmaker matchmaker_;
    SqliteUserRepository users_;
    std::unordered_map<std::string, int> ratings_; // player_id -> rating, cached at login
    std::unordered_map<std::string, std::vector<Event>> pending_events_; // room_id -> events since last broadcast
    std::unordered_set<std::string> subscribed_rooms_;

    // Every room reaches here at least once before its first tick (created
    // via JOIN_ROOM/QUICK_MATCH, or already existing when a new player joins
    // it) — subscribing here, once, is what feeds pending_events_ and rating
    // updates for that room from then on.
    void ensure_subscribed(GameRoom& room) {
        if (!subscribed_rooms_.insert(room.id()).second) {
            return;
        }
        std::string room_id = room.id();
        room.event_bus().subscribe([this, room_id](const Event& event) {
            pending_events_[room_id].push_back(event);
            if (std::holds_alternative<CheckmateEvent>(event)) {
                if (GameRoom* room_ptr = games_.find(room_id)) {
                    apply_rating_for_loss(*room_ptr, std::get<CheckmateEvent>(event).losing_color);
                }
            }
        });
    }

    void send_result(int connection_id, const std::string& type, bool success) {
        server_.send_text(connection_id,
            R"({"type":")" + type + R"(","success":)" + (success ? "true" : "false") + "}");
    }

    void handle_disconnect(int connection_id) {
        auto player_id = connections_.forget(connection_id);
        if (!player_id.has_value()) {
            return;
        }
        if (GameRoom* room = games_.find_room_for_player(*player_id)) {
            room->on_disconnect(*player_id);
        }
    }

    void handle_message(int connection_id, const std::string& raw) {
        ClientMessage message;
        try {
            message = WireFormat::client_message_from_json(raw);
        } catch (const WireFormat::WireFormatError&) {
            return; // malformed/unrecognized — silently ignored, matching CommandProcessor's convention
        }
        std::visit([this, connection_id](auto&& msg) { handle(connection_id, msg); }, message);
    }

    void handle(int connection_id, const RegisterMessage& msg) {
        send_result(connection_id, "register_result", users_.register_user(msg.username, msg.password));
    }

    void handle(int connection_id, const LoginMessage& msg) {
        auto record = users_.authenticate(msg.username, msg.password);
        if (record.has_value()) {
            connections_.identify(connection_id, msg.username);
            ratings_[msg.username] = record->rating;
            // Login is this protocol's reconnect signal (§6): cancel any
            // disconnect countdown still running for this player_id from a
            // prior connection on the same room.
            if (GameRoom* room = games_.find_room_for_player(msg.username)) {
                room->on_reconnect(msg.username);
            }
        }
        send_result(connection_id, "login_result", record.has_value());
    }

    void handle(int connection_id, const JoinRoomMessage& msg) {
        auto player_id = connections_.player_for(connection_id);
        if (!player_id.has_value()) {
            return; // must be logged in first
        }
        GameRoom& room = games_.get_or_create(msg.room_name);
        ensure_subscribed(room);
        room.join(*player_id);
    }

    void handle(int connection_id, const QuickMatchMessage&) {
        auto player_id = connections_.player_for(connection_id);
        if (!player_id.has_value()) {
            return;
        }
        auto rating_it = ratings_.find(*player_id);
        int rating = rating_it != ratings_.end() ? rating_it->second : RatingManager::kStartingRating;

        auto room_id = matchmaker_.find_quick_match(*player_id, rating);
        if (room_id.has_value()) {
            if (GameRoom* room = games_.find(*room_id)) {
                ensure_subscribed(*room);
            }
        }
        // No match yet: player_id is queued in the Matchmaker; §5's "no
        // suitable opponent" fallback message is left to the client to
        // surface after a timeout of its own — no separate error response
        // is wired up for this in the current pass.
    }

    void handle(int connection_id, const ResignMessage&) {
        auto player_id = connections_.player_for(connection_id);
        if (!player_id.has_value()) {
            return;
        }
        if (GameRoom* room = games_.find_room_for_player(*player_id)) {
            room->resign(*player_id);
        }
    }

    void handle(int connection_id, const MoveCommand& cmd) {
        dispatch_game_command(connection_id, Command{ cmd }, cmd.start);
    }

    void handle(int connection_id, const JumpCommand& cmd) {
        dispatch_game_command(connection_id, Command{ cmd }, cmd.cell);
    }

    // Anti-cheat gate (§4.1): the color of the piece at the command's source
    // cell must match the sending client's assigned color in this room.
    void dispatch_game_command(int connection_id, const Command& cmd, Position source_cell) {
        auto player_id = connections_.player_for(connection_id);
        if (!player_id.has_value()) {
            return;
        }
        GameRoom* room = games_.find_room_for_player(*player_id);
        if (!room) {
            return;
        }
        auto sender_color = room->color_of_player(*player_id);
        auto piece_color = room->engine().color_at(source_cell);
        if (!sender_color.has_value() || !piece_color.has_value() || *sender_color != *piece_color) {
            return;
        }
        CommandDispatcher::dispatch(cmd, room->engine());
    }

    // True white/black king presence lets a normal king-capture ending be
    // detected without GameEngine exposing "who lost" directly; resign() and
    // tick()'s disconnect-timeout forfeit leave both kings standing, so this
    // never double-fires for those (they publish their own CheckmateEvent).
    std::optional<Color> infer_losing_color_from_missing_king(const GameRoom& room) {
        bool white_king_alive = false;
        bool black_king_alive = false;
        for (const auto& piece : room.engine().piece_display_states()) {
            if (piece.type == PieceType::K) {
                (piece.color == Color::w ? white_king_alive : black_king_alive) = true;
            }
        }
        if (white_king_alive == black_king_alive) {
            return std::nullopt;
        }
        return white_king_alive ? Color::b : Color::w;
    }

    void apply_rating_for_loss(GameRoom& room, Color losing_color) {
        auto white_id = room.white_player_id();
        auto black_id = room.black_player_id();
        if (!white_id.has_value() || !black_id.has_value()) {
            return; // no opponent-vs-opponent match to rate (e.g. a lone player's room)
        }
        std::string loser_id = losing_color == Color::w ? *white_id : *black_id;
        std::string winner_id = losing_color == Color::w ? *black_id : *white_id;

        auto rating_of = [this](const std::string& player_id) {
            auto it = ratings_.find(player_id);
            return it != ratings_.end() ? it->second : RatingManager::kStartingRating;
        };
        auto [new_winner, new_loser] = RatingManager::apply_result(rating_of(winner_id), rating_of(loser_id));

        ratings_[winner_id] = new_winner;
        ratings_[loser_id] = new_loser;
        users_.update_rating(winner_id, new_winner);
        users_.update_rating(loser_id, new_loser);
    }

    void tick_rooms() {
        for (const auto& room_id : games_.room_ids()) {
            GameRoom* room = games_.find(room_id);
            if (!room) {
                continue;
            }
            ensure_subscribed(*room);

            bool was_over = room->engine().game_over();
            room->engine().wait(kTickMs);
            room->tick(kTickMs);

            if (!was_over && room->engine().game_over()) {
                if (auto losing_color = infer_losing_color_from_missing_king(*room)) {
                    room->event_bus().publish(CheckmateEvent{ *losing_color });
                }
            }

            auto& events = pending_events_[room->id()];
            std::string state_json = GameStateSerializer::to_json(room->engine(), events);
            events.clear();

            for (const auto& player_id : room->all_players()) {
                connections_.send_to_player(player_id, state_json);
            }
        }
    }
};

} // namespace

int main() {
    try {
        Server server;
        server.run();
    } catch (const std::exception& e) {
        std::cerr << "Fatal: " << e.what() << "\n";
        return 1;
    }
}
