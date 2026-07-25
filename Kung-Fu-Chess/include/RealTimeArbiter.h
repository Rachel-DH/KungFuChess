#pragma once

#include <vector>

#include "Board.h"
#include "Position.h"
#include "RuleEngine.h"

// Owns the simulated clock and all in-flight piece movement.
//
// Each advance() call is one simulation tick.  Every PendingMove whose
// next_step_ms has been reached takes exactly one cell step along its path,
// using the same logic for every step including the final one.  After all
// steps are applied, moves that have reached their destination are settled
// via RuleEngine (promotion, king-capture detection) and removed.
class RealTimeArbiter {
public:
    explicit RealTimeArbiter(long long move_ms_per_cell);

    // True if a pending move currently occupies (x, y).
    bool is_moving(int x, int y) const;

    bool is_airborne(int x, int y) const { return airborne_at(x, y) != nullptr; }

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

    bool has_activity() const { return !pending_moves_.empty() || !airborne_.empty(); }

    // True if the proposed route shares any cell with an already-pending route.
    bool conflicts_with_pending_move(int start_x, int start_y, int dest_x, int dest_y) const;

private:
    struct AirbornePiece {
        Position cell;
        Cell piece;
        long long land_ms;
    };

    long long move_ms_per_cell_;
    long long clock_ms_ = 0;
    std::vector<PendingMove> pending_moves_;
    std::vector<AirbornePiece> airborne_;

    const AirbornePiece* airborne_at(int x, int y) const;

    long long arrival_time_for(int start_x, int start_y, int dest_x, int dest_y) const;

    // Returns the next cell one step from `current` toward `dest` along a
    // straight or diagonal line (or `dest` itself for knight-style jumps).
    static Position next_cell_toward(Position current, Position dest);

    // Moves one piece one cell closer to its destination: clears current_cell,
    // places the piece on the next cell, updates current_cell and next_step_ms.
    // Works identically for intermediate and final steps.
    void step_move(PendingMove& move, Board& board) const;

    // Calls step_move for every pending move whose next_step_ms <= clock_ms_.
    void update_transit_positions(Board& board);

    // Settles moves that have reached dest (current_cell == dest) via
    // RuleEngine, removes them from pending_moves_, and expires landed jumps.
    // Returns true if an enemy king was captured.
    bool remove_completed_moves(Board& board);
};
