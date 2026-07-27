#include "RealTimeArbiter.h"

#include <algorithm>

namespace {

int abs_diff(int a, int b) {
    return a > b ? a - b : b - a;
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

// Order matters: airborne landings must be resolved before completed moves
// are settled, so a guard landing on the same tick an arriving move reaches
// that cell always wins the cell (see process_airborne_landings). Blocked
// moves are already gone by then — update_transit_positions removes them
// as soon as they're detected, with no dependency on this ordering.
bool RealTimeArbiter::advance(int milliseconds, Board& board) {
    if (milliseconds <= 0) {
        return false;
    }
    clock_ms_ += milliseconds;

    bool king_captured = update_transit_positions(board);

    if (process_airborne_landings(board)) {
        king_captured = true;
    }

    remove_completed_moves(board);

    return king_captured;
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

// Moves one piece one cell closer to its destination: clears current_cell,
// places the piece on the next cell, updates current_cell and next_step_ms.
// Works identically for intermediate and final steps — the same
// friendly-block check and the same king-capture check apply either way;
// there is no special case for "this is the last step".
// Sets king_captured (never clears it) if the piece landed on an enemy king
// this step. Returns false, without any mutation, if a friendly piece
// blocks the next cell — the caller (update_transit_positions) removes the
// move immediately in that case.
bool RealTimeArbiter::step_move(PendingMove& move, Board& board, bool& king_captured) const {
    Position next = next_cell_toward(move.current_cell, move.dest);

    if (RuleEngine::is_blocked_by_friendly(move.current_cell, next, board)) {
        return false;
    }

    if (RuleEngine::captures_enemy_king(next, move.piece.color, board)) {
        king_captured = true;
    }

    board.clear_at(move.current_cell.x, move.current_cell.y);
    board.place_at(next.x, next.y, move.piece);

    move.current_cell = next;
    move.next_step_ms += move_ms_per_cell_;
    return true;
}

// Calls step_move for every pending move whose next_step_ms <= clock_ms_,
// looping until it either runs out of due steps, reaches dest, or hits a
// friendly blocker. A blocked move is removed right here, immediately —
// the return value of step_move alone decides this, with no extra field
// needed on PendingMove. It is never retried on a later tick, and there's
// nothing to settle for it (no promotion, no capture happened).
// Moves that are still travelling, or have just reached dest, are left in
// place — arrivals are settled later, in remove_completed_moves, after
// airborne landings have had a chance to run (see advance()).
bool RealTimeArbiter::update_transit_positions(Board& board) {
    bool king_captured = false;

    pending_moves_.erase(
        std::remove_if(pending_moves_.begin(), pending_moves_.end(),
            [&](PendingMove& move) {
                while (move.next_step_ms <= clock_ms_ &&
                       (move.current_cell.x != move.dest.x || move.current_cell.y != move.dest.y)) {
                    if (!step_move(move, board, king_captured)) {
                        return true; // blocked by a friendly piece; remove now
                    }
                }
                return false; // still travelling, or just arrived
            }),
        pending_moves_.end());

    return king_captured;
}

// Resolves every airborne guard whose land_ms has been reached. Purely
// position-based: whatever enemy piece currently occupies the guard's cell
// is captured, regardless of whether it got there via a move still
// registered in pending_moves_ or is a plain static piece that was already
// sitting there. If it was mid-flight, the now-stale PendingMove is dropped
// too so it doesn't linger as a ghost entry. Every guard whose window has
// closed is removed here, whether or not it captured anything.
bool RealTimeArbiter::process_airborne_landings(Board& board) {
    bool king_captured = false;

    for (const AirbornePiece& guard : airborne_) {
        if (guard.land_ms > clock_ms_) {
            continue; // not landing yet
        }

        std::optional<Cell> occupant = board.get_at(guard.cell.x, guard.cell.y);
        if (occupant.has_value() && occupant->color != guard.piece.color) {
            if (occupant->type == PieceType::K) {
                king_captured = true;
            }
            board.clear_at(guard.cell.x, guard.cell.y);

            // Ghost-clean any PendingMove now claiming this cell, since its
            // piece was just captured out from under it. Bookkeeping only —
            // the capture above already happened regardless of this.
            pending_moves_.erase(
                std::remove_if(pending_moves_.begin(), pending_moves_.end(),
                    [&guard](const PendingMove& move) {
                        return move.current_cell == guard.cell;
                    }),
                pending_moves_.end());
        }
    }

    airborne_.erase(
        std::remove_if(airborne_.begin(), airborne_.end(),
            [this](const AirbornePiece& a) { return a.land_ms <= clock_ms_; }),
        airborne_.end());

    return king_captured;
}

// Settles every move that has reached its destination (current_cell == dest)
// via RuleEngine::settle_move (pawn promotion) and removes it from
// pending_moves_. King-capture was already detected earlier, uniformly, in
// step_move — there is nothing left to check for that here. Blocked moves
// never reach this function — update_transit_positions already removed them.
void RealTimeArbiter::remove_completed_moves(Board& board) {
    pending_moves_.erase(
        std::remove_if(pending_moves_.begin(), pending_moves_.end(),
            [&](const PendingMove& move) {
                if (move.current_cell.x != move.dest.x || move.current_cell.y != move.dest.y) {
                    return false; // still travelling
                }
                RuleEngine::settle_move(move, board);
                return true;
            }),
        pending_moves_.end());
}