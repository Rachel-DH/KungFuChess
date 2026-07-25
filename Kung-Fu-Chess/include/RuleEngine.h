#pragma once

#include "Board.h"
#include "Position.h"

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

// Centralizes move-validation helpers shared across Piece subclasses, and
// owns all board-mutation rules that apply when a move lands (king capture,
// pawn promotion).
class RuleEngine {
public:
    static bool captures_own_color(int start_x, int start_y, int dest_x, int dest_y, const Board& board);

    // Only meaningful when start–dest is a straight or diagonal line; callers must verify that first.
    static bool is_path_clear(int start_x, int start_y, int dest_x, int dest_y, const Board& board);

    // Applies the final landing rules to a move that has already been placed
    // on dest by the transit system: promotes pawns, detects king captures,
    // and re-writes the cell if the piece type changed.  Does NOT clear the
    // origin — transit already vacated it step by step.
    // Returns true if an enemy king was captured.
    static bool settle_move(const PendingMove& move, Board& board);

private:
    static bool captures_enemy_king(const PendingMove& move, const Board& board);
    static bool is_pawn_promotion(const PendingMove& move, const Board& board);
};
