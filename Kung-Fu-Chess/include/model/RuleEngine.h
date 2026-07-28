#pragma once

#include "model/Board.h"
#include "model/Position.h"

// A move that has been scheduled and is travelling toward its destination.
// Owned by RealTimeArbiter during transit.  current_cell tracks where the
// piece currently sits on the board; next_step_ms is the clock value at which
// the next single-cell step fires.
struct PendingMove {
    Position start;
    Position dest;
    Cell     piece;
    long long next_step_ms;  // clock time when the next cell step is due
    long long arrival_ms;    // clock time when the piece reaches dest
    Position  current_cell;  // cell the piece currently occupies on the board
};

// Centralizes move-validation and landing queries shared across Piece
// subclasses. Pawn promotion is applied here, in settle_move. King-capture
// is a stateless query (captures_enemy_king) — RuleEngine does not mutate
// the board for it; RealTimeArbiter does the actual clear/place.
class RuleEngine {
public:
    static bool captures_own_color(int start_x, int start_y, int dest_x, int dest_y, const Board& board);

    // True if the cell at `next` is occupied by a piece of the same color as the piece at `current`.
    static bool is_blocked_by_friendly(Position current, Position next, const Board& board);

    // True if the cell at `target` holds a king belonging to the color opposite `mover_color`.
    static bool captures_enemy_king(Position target, Color mover_color, const Board& board);

    // Only meaningful when start–dest is a straight or diagonal line; callers must verify that first.
    static bool is_path_clear(int start_x, int start_y, int dest_x, int dest_y, const Board& board);

    // Applies pawn promotion to a move that has already been placed on dest
    // by the transit system, re-writing the cell if the piece type changed.
    // Does NOT clear the origin — transit already vacated it step by step.
    // King-capture detection happens earlier and uniformly, in
    // RealTimeArbiter::step_move, on every step rather than only on arrival.
    static void settle_move(const PendingMove& move, Board& board);

private:
    static bool is_pawn_promotion(const PendingMove& move, const Board& board);
};
