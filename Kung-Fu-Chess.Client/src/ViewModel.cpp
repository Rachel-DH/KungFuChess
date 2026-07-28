#include "ViewModel.h"

#include <string>

#include "model/Board.h"
#include "model/Piece.h"

std::optional<std::string> ViewModel::on_click(Position cell, const ClientState& state) {
    if (!selected_.has_value()) {
        const PieceDisplayState* piece = state.piece_at(cell);
        if (piece && piece->color == my_color_ && piece->phase == PiecePhase::Idle) {
            selected_ = cell;
        }
        return std::nullopt;
    }

    Position start = *selected_;
    selected_.reset();

    if (start.x == cell.x && start.y == cell.y) {
        return std::nullopt; // clicking the same cell again just deselects
    }

    const PieceDisplayState* clicked_piece = state.piece_at(cell);
    if (clicked_piece && clicked_piece->color == my_color_ && clicked_piece->phase == PiecePhase::Idle) {
        selected_ = cell; // re-select a different one of my own pieces instead of attempting a move
        return std::nullopt;
    }

    if (!is_soft_legal(start, cell, state)) {
        return std::nullopt; // obviously illegal — don't bother the server with it
    }

    return R"({"type":"move","start":{"x":)" + std::to_string(start.x) + R"(,"y":)" + std::to_string(start.y)
        + R"(},"dest":{"x":)" + std::to_string(cell.x) + R"(,"y":)" + std::to_string(cell.y) + "}}";
}

bool ViewModel::is_soft_legal(Position start, Position dest, const ClientState& state) const {
    const PieceDisplayState* moving_piece = state.piece_at(start);
    if (!moving_piece) {
        return false;
    }

    Board board(8, 8);
    for (const auto& piece : state.pieces()) {
        board.place_at(piece.position.x, piece.position.y, Cell{ piece.color, piece.type });
    }

    const Piece* piece = PieceFactory::get_piece(moving_piece->type);
    return piece && piece->is_available_move(start.x, start.y, dest.x, dest.y, board);
}
