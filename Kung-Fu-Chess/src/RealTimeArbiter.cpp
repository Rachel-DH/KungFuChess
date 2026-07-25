#include "RealTimeArbiter.h"

#include <algorithm>

namespace {

int abs_diff(int a, int b) {
    return a > b ? a - b : b - a;
}

// Every cell a piece passes through from start to dest, inclusive.
// A non-straight, non-diagonal offset (e.g. a knight's L-shape) has nothing
// between, so the path is just the two endpoints.
std::vector<Position> path_cells(int start_x, int start_y, int dest_x, int dest_y) {
    int dx = abs_diff(start_x, dest_x);
    int dy = abs_diff(start_y, dest_y);

    if (dx != 0 && dy != 0 && dx != dy) {
        return { Position{ start_x, start_y }, Position{ dest_x, dest_y } };
    }

    int step_x = (dest_x > start_x) - (dest_x < start_x);
    int step_y = (dest_y > start_y) - (dest_y < start_y);

    std::vector<Position> cells;
    int x = start_x;
    int y = start_y;
    cells.push_back({ x, y });
    while (x != dest_x || y != dest_y) {
        x += step_x;
        y += step_y;
        cells.push_back({ x, y });
    }
    return cells;
}

} // namespace

// ---------------------------------------------------------------------------
// Construction
// ---------------------------------------------------------------------------

RealTimeArbiter::RealTimeArbiter(long long move_ms_per_cell)
    : move_ms_per_cell_(move_ms_per_cell) {
}

// ---------------------------------------------------------------------------
// Public queries
// ---------------------------------------------------------------------------

bool RealTimeArbiter::is_moving(int x, int y) const {
    for (const PendingMove& move : pending_moves_) {
        if (move.current_cell == Position{ x, y }) {
            return true;
        }
    }
    return false;
}

const RealTimeArbiter::AirbornePiece* RealTimeArbiter::airborne_at(int x, int y) const {
    for (const AirbornePiece& a : airborne_) {
        if (a.cell == Position{ x, y }) {
            return &a;
        }
    }
    return nullptr;
}

bool RealTimeArbiter::conflicts_with_pending_move(
    int start_x, int start_y, int dest_x, int dest_y) const
{
    std::vector<Position> new_path = path_cells(start_x, start_y, dest_x, dest_y);
    for (const PendingMove& move : pending_moves_) {
        std::vector<Position> existing = path_cells(
            move.start.x, move.start.y, move.dest.x, move.dest.y);
        for (const Position& a : new_path) {
            for (const Position& b : existing) {
                if (a == b) return true;
            }
        }
    }
    return false;
}

// ---------------------------------------------------------------------------
// Public commands
// ---------------------------------------------------------------------------

void RealTimeArbiter::drop_airborne_at(int x, int y) {
    airborne_.erase(
        std::remove_if(airborne_.begin(), airborne_.end(),
            [x, y](const AirbornePiece& a) { return a.cell.x == x && a.cell.y == y; }),
        airborne_.end());
}

void RealTimeArbiter::schedule_move(Position start, Position dest, Cell piece) {
    pending_moves_.push_back(PendingMove{
        start,
        dest,
        piece,
        clock_ms_ + move_ms_per_cell_,          // first step fires one cell-time from now
        arrival_time_for(start.x, start.y, dest.x, dest.y),
        start,                                   // piece currently sits at start
    });
}

void RealTimeArbiter::start_jump(Position cell, Cell piece, long long jump_duration_ms) {
    airborne_.push_back(AirbornePiece{ cell, piece, clock_ms_ + jump_duration_ms });
}

// ---------------------------------------------------------------------------
// Tick
// ---------------------------------------------------------------------------

bool RealTimeArbiter::advance(int milliseconds, Board& board) {
    if (milliseconds <= 0) {
        return false;
    }
    clock_ms_ += milliseconds;
    update_transit_positions(board);
    return remove_completed_moves(board);
}

// ---------------------------------------------------------------------------
// Private helpers
// ---------------------------------------------------------------------------

long long RealTimeArbiter::arrival_time_for(
    int start_x, int start_y, int dest_x, int dest_y) const
{
    int distance = std::max(abs_diff(start_x, dest_x), abs_diff(start_y, dest_y));
    return clock_ms_ + static_cast<long long>(distance) * move_ms_per_cell_;
}

// Returns the neighbour of `current` that is one step closer to `dest`.
// For a straight or diagonal path this is current + (sign_dx, sign_dy).
// For a knight jump (no intermediate cells) it returns dest directly.
Position next_cell_toward(Position current, Position dest) { 
    int dx = abs_diff(current.x, dest.x);
    int dy = abs_diff(current.y, dest.y); 
    if (dx != 0 && dy != 0 && dx != dy) { 
        return dest; 
    } 

    int step_x = (dest.x > current.x) - (dest.x < current.x); 
    int step_y = (dest.y > current.y) - (dest.y < current.y); 
    return Position{ current.x + step_x, current.y + step_y }; 
}

// Moves one piece exactly one cell along its path.  This is the single
// building block used for every step, intermediate or final.
//   1. Clear the cell the piece currently occupies.
//   2. Place the piece on the next cell (skip placement if already occupied,
//      which can only happen with an airborne guard — the arriving piece is
//      considered captured at that point and remove_completed_moves will
//      handle the guard interaction).
//   3. Update current_cell and schedule the following step.
void RealTimeArbiter::step_move(PendingMove& move, Board& board) const {
    Position next = next_cell_toward(move.current_cell, move.dest);

    board.clear_at(move.current_cell.x, move.current_cell.y);

    if (!board.get_at(next.x, next.y).has_value()) {
        board.place_at(next.x, next.y, move.piece);
    }

    move.current_cell = next;
    move.next_step_ms += move_ms_per_cell_;
}

// Advances every pending move that is due for a step this tick.
// A single advance() call may cover multiple cell-widths of time, so we loop
// until no more steps are due — each step schedules itself one cell-time ahead.
void RealTimeArbiter::update_transit_positions(Board& board) {
    for (PendingMove& move : pending_moves_) {
        while (move.next_step_ms <= clock_ms_ && move.current_cell != move.dest) {
            step_move(move, board);
        }
    }
}

// Settles every move that has reached its destination, handles the airborne
// guard interaction, then removes those moves from pending_moves_.
// Also expires airborne records whose window has closed.
bool RealTimeArbiter::remove_completed_moves(Board& board) {
    bool king_captured = false;

    pending_moves_.erase(
        std::remove_if(pending_moves_.begin(), pending_moves_.end(),
            [&](const PendingMove& move) {
                if (move.current_cell != move.dest) {
                    return false; // still travelling
                }

                // An airborne enemy on dest captures the arriving piece:
                // the piece was already placed on dest by step_move but the
                // guard wins — clear dest and leave the guard untouched.
                const AirbornePiece* guard = airborne_at(move.dest.x, move.dest.y);
                if (guard != nullptr && guard->piece.color != move.piece.color
                    && move.arrival_ms <= guard->land_ms)
                {
                    board.clear_at(move.dest.x, move.dest.y);
                    if (move.piece.type == PieceType::K) {
                        king_captured = true;
                    }
                    return true; // remove this move
                }

                // Normal arrival: apply promotion / king-capture detection.
                drop_airborne_at(move.dest.x, move.dest.y);
                if (RuleEngine::settle_move(move, board)) {
                    king_captured = true;
                }
                return true; // remove this move
            }),
        pending_moves_.end());

    // Expire airborne records whose guard window has closed.
    airborne_.erase(
        std::remove_if(airborne_.begin(), airborne_.end(),
            [this](const AirbornePiece& a) { return a.land_ms <= clock_ms_; }),
        airborne_.end());

    return king_captured;
}
