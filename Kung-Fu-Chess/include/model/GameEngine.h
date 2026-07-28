#pragma once

#include <iostream>
#include <optional>
#include <vector>

#include "model/Board.h"
#include "model/Position.h"
#include "control/RealTimeArbiter.h"
#include "utils/Types.h"

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
    static constexpr long long DEFAULT_MOVE_MS_PER_CELL = 1000;
    static constexpr long long JUMP_DURATION_MS = 1000;

    explicit GameEngine(Board board, long long move_ms_per_cell = DEFAULT_MOVE_MS_PER_CELL);

    // Fully-constructed GameEngine on the standard 8x8 chess starting position.
    static GameEngine standard_start(long long move_ms_per_cell = DEFAULT_MOVE_MS_PER_CELL);

    // The Board for the standard starting position, without wrapping it in a GameEngine — for callers that build their own engine and must never see GameEngine directly.
    static Board standard_start_board();

    // Validates the move against the piece's own rule and any move already in flight on its route, then queues it via RealTimeArbiter; false if illegal, game over, no piece at `start`, or already moving/airborne/resting.
    bool request_move(Position start, Position dest);

    // Starts a jump in place at `cell` for JUMP_DURATION_MS; false if the game is over, there's no piece there, or it's already moving/airborne.
    bool request_jump(Position cell);

    // Ends the game immediately regardless of board state (e.g. resignation); a no-op if already over.
    void force_game_over() { state_ = GameState::GameOver; }

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

    // True if `cell` holds a piece that can be selected: present, neither mid-move, mid-jump, nor resting, and the game is not already over.
    bool is_selectable(Position cell) const;

    std::optional<Color> color_at(Position cell) const;

    // Snapshot of every occupied cell, for rendering; independent of Board/Piece storage, and available even after the game is over.
    std::vector<PieceDisplayState> piece_display_states() const;

private:
    Board board_;
    RealTimeArbiter arbiter_;
    GameState state_ = GameState::Running;
};
