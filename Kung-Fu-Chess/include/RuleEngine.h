#pragma once

#include "Board.h"
#include "Position.h"

// A move that has been scheduled and is now arriving at its destination.
// Owned by RealTimeArbiter during transit; passed to RuleEngine::settle_move
// when arrival time is reached so that all rule logic lives in one place.
struct PendingMove {
    Position start;
    Position dest;
    Cell piece;
    long long arrival_ms;
};

// SRP: centralizes move-validation logic (self-capture, path-blocking) that
// would otherwise be duplicated across every Piece subclass, and owns all
// board-mutation rules that apply when a move lands (king capture, pawn
// promotion).
class RuleEngine {
public:
    static bool captures_own_color(int start_x, int start_y, int dest_x, int dest_y, const Board& board);

    // Only meaningful when start-dest is itself a straight or diagonal line; callers must check that first.
    static bool is_path_clear(int start_x, int start_y, int dest_x, int dest_y, const Board& board);

    // Applies the arriving move to the board: promotes pawns, records king
    // captures, and writes/clears cells.  Returns true if an enemy king was
    // captured by this move.  Does NOT check legality — that is the caller's
    // responsibility before scheduling the move.
    static bool settle_move(const PendingMove& move, Board& board);

private:
    // True if `move` captures the enemy king currently sitting on its destination.
    static bool captures_enemy_king(const PendingMove& move, const Board& board);

    // True if `move`'s piece is a pawn reaching the far rank (promotion row).
    static bool is_pawn_promotion(const PendingMove& move, const Board& board);
};
