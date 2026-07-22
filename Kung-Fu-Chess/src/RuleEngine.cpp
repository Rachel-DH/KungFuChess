#include "RuleEngine.h"

bool RuleEngine::captures_own_color(int start_x, int start_y, int dest_x, int dest_y, const Board& board) {
    std::optional<Cell> dest_cell = board.get_at(dest_x, dest_y);
    if (!dest_cell.has_value()) {
        return false;
    }
    std::optional<Cell> start_cell = board.get_at(start_x, start_y);
    return start_cell.has_value() && start_cell->color == dest_cell->color;
}

bool RuleEngine::is_path_clear(int start_x, int start_y, int dest_x, int dest_y, const Board& board) {
    int step_x = (dest_x > start_x) - (dest_x < start_x);
    int step_y = (dest_y > start_y) - (dest_y < start_y);

    int x = start_x + step_x;
    int y = start_y + step_y;

    while (x != dest_x || y != dest_y) {
        if (board.get_at(x, y).has_value()) {
            return false;
        }
        x += step_x;
        y += step_y;
    }
    return true;
}

// private ---------------------------------------------------------------

bool RuleEngine::captures_enemy_king(const PendingMove& move, const Board& board) {
    std::optional<Cell> target = board.get_at(move.dest.x, move.dest.y);
    return target.has_value() && target->type == PieceType::K && target->color != move.piece.color;
}

// Farthest row is row 0 for white (which moves up toward lower indices),
// the last row for black.
bool RuleEngine::is_pawn_promotion(const PendingMove& move, const Board& board) {
    if (move.piece.type != PieceType::P) {
        return false;
    }
    int last_row = (move.piece.color == Color::w) ? 0 : board.get_height() - 1;
    return move.dest.y == last_row;
}

// public ----------------------------------------------------------------

bool RuleEngine::settle_move(const PendingMove& move, Board& board) {
    bool king_captured = captures_enemy_king(move, board);

    Cell piece = move.piece;
    if (is_pawn_promotion(move, board)) {
        piece.type = PieceType::Q;
    }

    board.place_at(move.dest.x, move.dest.y, piece);
    board.clear_at(move.start.x, move.start.y);

    return king_captured;
}
