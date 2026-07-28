#include "GameStateSerializer.h"

#include "ThirdParty/nlohmann/json.hpp"

namespace GameStateSerializer {

namespace {

using nlohmann::json;

char color_letter(Color color) {
    return color == Color::w ? 'w' : 'b';
}

const char* type_letter(PieceType type) {
    switch (type) {
        case PieceType::K: return "K";
        case PieceType::Q: return "Q";
        case PieceType::R: return "R";
        case PieceType::B: return "B";
        case PieceType::N: return "N";
        case PieceType::P: return "P";
    }
    return "?";
}

const char* phase_name(PiecePhase phase) {
    switch (phase) {
        case PiecePhase::Idle: return "idle";
        case PiecePhase::Move: return "move";
        case PiecePhase::Jump: return "jump";
    }
    return "idle";
}

json event_to_json(const Event& event) {
    return std::visit(
        [](const auto& e) -> json {
            using T = std::decay_t<decltype(e)>;
            if constexpr (std::is_same_v<T, CaptureEvent>) {
                return json{
                    { "type", "capture" },
                    { "at", { { "x", e.at.x }, { "y", e.at.y } } },
                    { "captured_type", type_letter(e.captured_type) },
                    { "captured_color", std::string(1, color_letter(e.captured_color)) },
                };
            } else if constexpr (std::is_same_v<T, CheckmateEvent>) {
                return json{
                    { "type", "game_over" },
                    { "losing_color", std::string(1, color_letter(e.losing_color)) },
                };
            } else {
                return json{
                    { "type", "disconnect" },
                    { "player_id", e.player_id },
                };
            }
        },
        event);
}

} // namespace

std::string to_json(const GameEngine& engine, const std::vector<Event>& events) {
    json pieces = json::array();
    for (const auto& piece : engine.piece_display_states()) {
        pieces.push_back(json{
            { "id", piece.id },
            { "x", piece.position.x },
            { "y", piece.position.y },
            { "type", type_letter(piece.type) },
            { "color", std::string(1, color_letter(piece.color)) },
            { "phase", phase_name(piece.phase) },
        });
    }

    json event_array = json::array();
    for (const auto& event : events) {
        event_array.push_back(event_to_json(event));
    }

    json root{
        { "type", "game_state" },
        { "pieces", pieces },
        { "game_over", engine.game_over() },
        { "events", event_array },
    };
    return root.dump();
}

} // namespace GameStateSerializer
