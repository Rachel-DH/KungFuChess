#pragma once

#include <vector>

#include "Board.h"
#include "Constants.h"
#include "Position.h"
#include "RuleEngine.h"

// Owns the simulated clock and all in-flight piece movement.
//
// Each advance() call is one simulation tick.  Every PendingMove whose
// next_step_ms has been reached takes exactly one cell step along its path,
// using the same logic for every step including the final one; a king
// captured on any step — mid-path or on arrival — ends the game immediately.
// A move that finds a friendly piece occupying its next cell is removed
// immediately, permanently, with nothing settled — it is never retried on a
// later tick. After all steps are applied, moves that have reached their
// destination are settled via RuleEngine (pawn promotion) and removed.
class RealTimeArbiter {
public:
    explicit RealTimeArbiter(long long move_ms_per_cell,
        long long rest_duration_ms = constants::kDefaultRestDurationMs);

    // True if a pending move currently occupies (x, y).
    bool is_moving(int x, int y) const;

    bool is_airborne(int x, int y) const { return airborne_at(x, y) != nullptr; }

    // True if a piece settled at (x, y) is still within its post-move rest window.
    bool is_resting(int x, int y) const;

    // Removes the airborne record for (x, y) so it never outlives the piece it describes.
    void drop_airborne_at(int x, int y);

    // Queues a new move; the piece takes its first step on the next tick whose
    // clock reaches next_step_ms.
    void schedule_move(Position start, Position dest, Cell piece);

    // Guards cell for jump_duration_ms: an enemy arriving there during the
    // window is captured by the jumper instead of landing normally.
    void start_jump(Position cell, Cell piece, long long jump_duration_ms);

    // Advances the clock.  Every pending move due for a step takes one cell
    // step; completed moves are settled and removed.
    // Returns true if an enemy king was captured this tick.
    bool advance(int milliseconds, Board& board);

    long long clock_ms() const { return clock_ms_; }

    bool has_activity() const {
        return !pending_moves_.empty() || !airborne_.empty() || !resting_.empty();
    }

private:
    struct AirbornePiece {
        Position cell;
        Cell piece;
        long long land_ms;
    };

    struct RestingPiece {
        Position cell;
        long long rest_until_ms;
    };

    long long move_ms_per_cell_;
    long long rest_duration_ms_;
    long long clock_ms_ = 0;
    std::vector<PendingMove> pending_moves_;
    std::vector<AirbornePiece> airborne_;
    std::vector<RestingPiece> resting_;

    const AirbornePiece* airborne_at(int x, int y) const;

    // Starts (or restarts) cell's rest window from now, discarding any
    // pre-existing record for it first so a capturing piece always gets its
    // own fresh window rather than inheriting the victim's.
    void start_resting(Position cell);

    // Removes every resting record whose window has closed.
    void expire_resting();

    long long arrival_time_for(int start_x, int start_y, int dest_x, int dest_y) const;

    // Moves one piece one cell closer to its destination: clears current_cell,
    // places the piece on the next cell, updates current_cell and next_step_ms.
    // Works identically for intermediate and final steps.  Sets king_captured
    // (never clears it) if the piece landed on an enemy king this step.
    // Returns false, without any mutation, if a friendly piece occupies the
    // next cell — update_transit_positions treats that as "remove this move
    // now, permanently", so no extra state needs to live on PendingMove itself.
    bool step_move(PendingMove& move, Board& board, bool& king_captured) const;

    // Calls step_move for every pending move whose next_step_ms <= clock_ms_,
    // looping until it either runs out of due steps, reaches dest, or is
    // blocked. A blocked move is removed immediately, right here — it is
    // never retried on a later tick, and there is nothing to settle for it.
    // Returns true if any step captured an enemy king this tick.
    bool update_transit_positions(Board& board);

    // Resolves every airborne guard whose land_ms has been reached: an enemy
    // occupying the guard's cell is captured (clearing the cell, and
    // ghost-cleaning any PendingMove now claiming that cell, since its piece
    // was just captured out from under it); a friendly or absent occupant is
    // left untouched. The guard is removed either way. Runs before
    // remove_completed_moves so a guard landing on the same tick an arriving
    // move reaches that cell always wins the cell.
    // Returns true if an enemy king was captured.
    bool process_airborne_landings(Board& board);

    // Removes moves that have reached dest, settling each via RuleEngine
    // first (pawn promotion). Blocked moves never reach here — they're
    // already removed by update_transit_positions.
    void remove_completed_moves(Board& board);
};