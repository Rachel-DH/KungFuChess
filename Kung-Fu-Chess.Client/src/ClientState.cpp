#include "ClientState.h"

#include "ThirdParty/nlohmann/json.hpp"

namespace {

using nlohmann::json;

PieceType type_from_letter(const std::string& letter) {
    if (letter == "K") return PieceType::K;
    if (letter == "Q") return PieceType::Q;
    if (letter == "R") return PieceType::R;
    if (letter == "B") return PieceType::B;
    if (letter == "N") return PieceType::N;
    return PieceType::P;
}

Color color_from_letter(const std::string& letter) {
    return letter == "w" ? Color::w : Color::b;
}

PiecePhase phase_from_name(const std::string& name) {
    if (name == "move") return PiecePhase::Move;
    if (name == "jump") return PiecePhase::Jump;
    return PiecePhase::Idle;
}

} // namespace

void ClientState::apply_game_state_json(const std::string& json_text) {
    json root;
    try {
        root = json::parse(json_text);
    } catch (const json::parse_error&) {
        return; // malformed message — keep the last known-good snapshot
    }
    if (!root.is_object() || !root.contains("pieces") || !root.contains("game_over")) {
        return;
    }

    std::vector<PieceDisplayState> parsed_pieces;
    for (const auto& piece_json : root["pieces"]) {
        PiecePhase phase = phase_from_name(piece_json.value("phase", "idle"));
        parsed_pieces.push_back(PieceDisplayState{
            piece_json.value("id", 0),
            Position{ piece_json.value("x", 0), piece_json.value("y", 0) },
            type_from_letter(piece_json.value("type", "P")),
            color_from_letter(piece_json.value("color", "w")),
            phase == PiecePhase::Move,
            phase == PiecePhase::Jump,
            phase,
        });
    }

    pieces_ = std::move(parsed_pieces);
    game_over_ = root.value("game_over", false);
    has_state_ = true;
}

const PieceDisplayState* ClientState::piece_at(Position cell) const {
    for (const auto& piece : pieces_) {
        if (piece.position.x == cell.x && piece.position.y == cell.y) {
            return &piece;
        }
    }
    return nullptr;
}
