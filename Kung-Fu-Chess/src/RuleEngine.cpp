#include "RuleEngine.h"

bool RuleEngine::captures_own_color(int start_x, int start_y, int dest_x, int dest_y, const Board& board) {
    std::optional<Cell> dest_cell = board.get_at(dest_x, dest_y);
    if (!dest_cell.has_value()) {
        return false;
    }
    std::optional<Cell> start_cell = board.get_at(start_x, start_y);
    return start_cell.has_value() && start_cell->color == dest_cell->color;
}

bool RuleEngine::is_blocked_by_friendly(Position current, Position next, const Board& board) {
    std::optional<Cell> occupant = board.get_at(next.x, next.y);
    std::optional<Cell> moving_piece = board.get_at(current.x, current.y);

    if (occupant.has_value() && moving_piece.has_value()) {
        return occupant->color == moving_piece->color;
    }
    return false;
}

bool RuleEngine::captures_enemy_king(Position target, Color mover_color, const Board& board) {
    std::optional<Cell> occupant = board.get_at(target.x, target.y);
    return occupant.has_value() && occupant->type == PieceType::K && occupant->color != mover_color;
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

void RuleEngine::settle_move(const PendingMove& move, Board& board) {
    Cell piece = move.piece;
    if (is_pawn_promotion(move, board)) {
        piece.type = PieceType::Q;
    }

    // The piece was already placed on dest step-by-step during transit.
    // Re-write the cell only to apply any type change (e.g. pawn promotion).
    // Do NOT clear move.start — transit vacated the origin long ago.
    board.place_at(move.dest.x, move.dest.y, piece);
}
