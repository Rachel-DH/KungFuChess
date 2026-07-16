#pragma once

#include <iostream>
#include <optional>
#include <vector>

#include "Board.h"
#include "Constants.h"
#include "Position.h"
#include "RealTimeArbiter.h"
#include "Types.h"

enum class GameState { Running, GameOver };

// Decoupled snapshot of one occupied cell, for the UI to render without
// touching Board/Piece internals.
struct PieceDisplayState {
    Position position;
    PieceType type;
    Color color;
    bool is_moving;
    bool is_airborne;
};

// Facade: coordinates Board, RealTimeArbiter, and the Piece/RuleEngine move
// rules behind one simple move/jump/wait/print interface.
class GameEngine {
public:
    static constexpr long long kDefaultMoveMsPerCell = constants::kDefaultMoveMsPerCell;
    static constexpr long long kJumpDurationMs = constants::kJumpDurationMs;

    explicit GameEngine(Board board, long long move_ms_per_cell = kDefaultMoveMsPerCell);

    // Validates the move against the piece's own rule and against any move
    // already in flight on its route, then queues it via RealTimeArbiter.
    // False (board unchanged) if illegal, the game is over, there's no
    // piece at `start`, or it's already moving or airborne. An out-of-range
    // `start`/`dest` propagates Board's std::out_of_range, provided the game
    // isn't already over.
    bool request_move(Position start, Position dest);

    // Starts a jump in place at `cell` for kJumpDurationMs. False if the
    // game is over, there's no piece there, or it's already moving/airborne.
    // An out-of-range `cell` propagates Board's std::out_of_range, provided
    // the game isn't already over.
    bool request_jump(Position cell);

    // Advances the game clock and settles any pending moves whose arrival
    // time has now passed.
    void wait(int milliseconds);

    // Prints the settled board; pieces mid-move still show at their origin.
    void print(std::ostream& out) const;

    long long clock_ms() const { return arbiter_.clock_ms(); }
    GameState state() const { return state_; }
    bool game_over() const { return state_ == GameState::GameOver; }

    int width() const { return board_.get_width(); }
    int height() const { return board_.get_height(); }

    // True if `cell` holds a piece that can be selected: present, neither
    // mid-move nor mid-jump, and the game is not already over.
    bool is_selectable(Position cell) const;

    std::optional<Color> color_at(Position cell) const;

    // Snapshot of every occupied cell, for rendering. Independent of
    // Board/Piece storage, and available even after the game is over.
    std::vector<PieceDisplayState> piece_display_states() const;

private:
    Board board_;
    RealTimeArbiter arbiter_;
    GameState state_ = GameState::Running;
};
