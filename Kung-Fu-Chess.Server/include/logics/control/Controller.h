#pragma once

#include <iostream>
#include <optional>

#include "model/GameEngine.h"
#include "model/Position.h"

// SRP: translates logical-cell clicks into GameEngine move/jump requests and owns the UI-facing
// selection state; also the sole owner of the GameEngine, which callers (main) never see directly.
// Pixel coordinates are never seen here — converting a mouse click to a Position (or determining it
// missed the board entirely, in which case the caller calls deselect() instead) is the caller's job.
class Controller {
public:
    explicit Controller(Board board, long long move_ms_per_cell = GameEngine::DEFAULT_MOVE_MS_PER_CELL);

    // Fully-constructed Controller on the standard 8x8 chess starting position.
    static Controller standard_start(long long move_ms_per_cell = GameEngine::DEFAULT_MOVE_MS_PER_CELL);

    // Selects a piece, reselects onto another friendly piece, or requests a move of the current
    // selection; a rejected move leaves the selection in place. A cell outside the board is treated
    // the same as deselect() (defensive: a caller should normally filter this out before calling).
    void click(Position cell);

    // Starts a jump at cell; drops the selection if the jumped piece was the current selection, since
    // it can no longer be moved. A cell outside the board is silently ignored, same as before.
    void jump(Position cell);

    // Clears the current selection; the caller's stand-in for "the click didn't land on the board".
    void deselect();

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
    int width() const { return engine_.width(); }
    int height() const { return engine_.height(); }

private:
    GameEngine engine_;
    std::optional<Position> selected_;

    // Returns false only when the selected cell turned out to be stale (its piece is gone); the selection is cleared so the click can be retried as a fresh one.
    bool handle_click_with_selection(Position cell, std::optional<Color> clicked_color, bool clicked_cell_is_selectable);

    bool in_bounds(Position cell) const {
        return cell.x >= 0 && cell.y >= 0 && cell.x < engine_.width() && cell.y < engine_.height();
    }
};
