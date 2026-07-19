#include "piece_naming.h"

#include <stdexcept>

namespace piece_naming {

std::string phase_dir_name(PiecePhase phase) {
    switch (phase) {
        case PiecePhase::Idle: return "idle";
        case PiecePhase::Move: return "move";
        case PiecePhase::Jump: return "jump";
    }
    throw std::invalid_argument("Unknown PiecePhase");
}

char type_letter(PieceType type) {
    switch (type) {
        case PieceType::K: return 'K';
        case PieceType::Q: return 'Q';
        case PieceType::R: return 'R';
        case PieceType::B: return 'B';
        case PieceType::N: return 'N';
        case PieceType::P: return 'P';
    }
    throw std::invalid_argument("Unknown PieceType");
}

std::string piece_code(PieceType type, Color color) {
    char color_letter = (color == Color::w) ? 'W' : 'B';
    return std::string(1, type_letter(type)) + color_letter;
}

}  // namespace piece_naming
