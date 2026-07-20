#pragma once

#include <iostream>
#include <optional>
#include <vector>

#include "Board.h"
#include "Constants.h"
#include "Position.h"
#include "RealTimeArbiter.h"
#include "Types.h"

enum class GameState
{
    Running,
    GameOver
};

// Decoupled snapshot of one occupied cell, for the UI to render without touching Board/Piece internals.
struct PieceDisplayState
{
    int id; // stable for one occupied cell across a single move/jump in flight; NOT a persistent piece identity — a capture landing on the same cell immediately after can reuse it
    Position position;
    PieceType type;
    Color color;
    bool is_moving;
    bool is_airborne;
    PiecePhase phase;
};

// Facade: coordinates Board, RealTimeArbiter, and the Piece/RuleEngine move rules behind one simple move/jump/wait/print interface.
class GameEngine
{
public:
    static constexpr long long kDefaultMoveMsPerCell = constants::kDefaultMoveMsPerCell;
    static constexpr long long kJumpDurationMs = constants::kJumpDurationMs;

    explicit GameEngine(Board board, long long move_ms_per_cell = kDefaultMoveMsPerCell);

    // Fully-constructed GameEngine on the standard 8x8 chess starting position.
    static GameEngine standard_start(long long move_ms_per_cell = kDefaultMoveMsPerCell);

    // The Board for the standard starting position, without wrapping it in a GameEngine — for callers that build their own engine and must never see GameEngine directly.
    static Board standard_start_board();

    // Validates the move against the piece's own rule and any move already in flight on its route, then queues it via RealTimeArbiter; false if illegal, game over, no piece at `start`, or already moving/airborne.
    bool request_move(Position start, Position dest);

    // Starts a jump in place at `cell` for kJumpDurationMs; false if the game is over, there's no piece there, or it's already moving/airborne.
    bool request_jump(Position cell);

    // Advances the game clock and settles any pending moves whose arrival time has now passed.
    void wait(int milliseconds);

    // Prints the settled board; pieces mid-move still show at their origin.
    void print(std::ostream &out) const;

    long long clock_ms() const { return arbiter_.clock_ms(); }
    GameState state() const { return state_; }
    bool game_over() const { return state_ == GameState::GameOver; }
    bool has_activity() const { return arbiter_.has_activity(); }

    int width() const { return board_.get_width(); }
    int height() const { return board_.get_height(); }

    // True if `cell` holds a piece that can be selected: present, neither mid-move nor mid-jump, and the game is not already over.
    bool is_selectable(Position cell) const;

    std::optional<Color> color_at(Position cell) const;

    // Snapshot of every occupied cell, for rendering; independent of Board/Piece storage, and available even after the game is over.
    std::vector<PieceDisplayState> piece_display_states() const;

private:
    Board board_;
    RealTimeArbiter arbiter_;
    GameState state_ = GameState::Running;
};
