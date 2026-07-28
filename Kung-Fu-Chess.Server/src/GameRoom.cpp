#include "GameRoom.h"

#include <algorithm>

GameRoom::GameRoom(std::string id)
    : id_(std::move(id)), engine_(GameEngine::standard_start()) {
}

std::optional<Color> GameRoom::color_of(const std::string& player_id) const {
    if (white_player_ == player_id) {
        return Color::w;
    }
    if (black_player_ == player_id) {
        return Color::b;
    }
    return std::nullopt;
}

JoinResult GameRoom::join(const std::string& player_id) {
    if (white_player_ == player_id) {
        return { PlayerRole::Opponent, Color::w };
    }
    if (black_player_ == player_id) {
        return { PlayerRole::Opponent, Color::b };
    }
    if (!white_player_.has_value()) {
        white_player_ = player_id;
        return { PlayerRole::Opponent, Color::w };
    }
    if (!black_player_.has_value()) {
        black_player_ = player_id;
        return { PlayerRole::Opponent, Color::b };
    }
    spectators_.push_back(player_id);
    return { PlayerRole::Spectator, std::nullopt };
}

void GameRoom::on_disconnect(const std::string& player_id) {
    if (!color_of(player_id).has_value()) {
        return; // spectators don't hold a slot to forfeit
    }
    on_reconnect(player_id); // drop any stale timer so this one starts fresh (§6: resets on every disconnect)
    disconnect_timers_.push_back({ player_id, kDisconnectTimeoutMs });
    event_bus_.publish(PlayerDisconnectedEvent{ player_id });
}

void GameRoom::on_reconnect(const std::string& player_id) {
    disconnect_timers_.erase(
        std::remove_if(disconnect_timers_.begin(), disconnect_timers_.end(),
            [&](const DisconnectTimer& timer) { return timer.player_id == player_id; }),
        disconnect_timers_.end());
}

void GameRoom::resign(const std::string& player_id) {
    std::optional<Color> resigning_color = color_of(player_id);
    if (!resigning_color.has_value()) {
        return;
    }
    engine_.force_game_over();
    disconnect_timers_.clear();
    event_bus_.publish(CheckmateEvent{ *resigning_color });
}

std::optional<Color> GameRoom::tick(long long elapsed_ms) {
    for (auto& timer : disconnect_timers_) {
        timer.remaining_ms -= elapsed_ms;
        if (timer.remaining_ms <= 0) {
            std::optional<Color> losing_color = color_of(timer.player_id);
            engine_.force_game_over();
            disconnect_timers_.clear();
            if (losing_color.has_value()) {
                event_bus_.publish(CheckmateEvent{ *losing_color });
            }
            return losing_color;
        }
    }
    return std::nullopt;
}
