#pragma once

#include <iostream>
#include <optional>

#include "GameEngine.h"
#include "Position.h"

// SRP: translates pixel clicks into GameEngine move/jump requests and owns the UI-facing selection state; also the sole owner of the GameEngine, which callers (main) never see directly.
class Controller {
public:
    explicit Controller(Board board, long long move_ms_per_cell = GameEngine::kDefaultMoveMsPerCell);

    // Fully-constructed Controller on the standard 8x8 chess starting position.
    static Controller standard_start(long long move_ms_per_cell = GameEngine::kDefaultMoveMsPerCell);

    // Selects a piece, reselects onto another friendly piece, or requests a move of the current selection; a click outside the board cancels the selection, and a rejected move leaves it in place.
    void click(int pixel_x, int pixel_y);

    // Starts a jump at the clicked cell; drops the selection if the jumped piece was the current selection, since it can no longer be moved.
    void jump(int pixel_x, int pixel_y);

    // Advances the game clock and settles any pending moves whose arrival time has now passed.
    void wait(int milliseconds);

    // Prints the settled board; pieces mid-move still show at their origin.
    void print(std::ostream& out) const;

    // Snapshot of every occupied cell, for rendering; independent of Board/Piece storage, and available even after the game is over.
    std::vector<PieceDisplayState> piece_display_states() const;

    bool has_selection() const { return selected_.has_value(); }
    std::optional<Position> selected() const { return selected_; }
    bool game_over() const { return engine_.game_over(); }
    bool has_activity() const { return engine_.has_activity(); }

private:
    GameEngine engine_;
    std::optional<Position> selected_;

    // Returns false only when the selected cell turned out to be stale (its piece is gone); the selection is cleared so the click can be retried as a fresh one.
    bool handle_click_with_selection(Position cell, std::optional<Color> clicked_color, bool clicked_cell_is_selectable);
};
