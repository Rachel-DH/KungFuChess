#pragma once

#include <optional>

#include "Controller.h"
#include "IInputSource.h"
#include "IRenderer.h"
#include "Position.h"
#include "RenderSnapshot.h"

// Adapter: drives one windowed frame — advance the clock, apply a pending click, and redraw only
// when something actually changed — the windowed counterpart to CommandProcessor's per-line text
// protocol; owns none of Controller/IRenderer/IInputSource.
class RenderLoop {
public:
    RenderLoop(Controller& controller, IRenderer& renderer, IInputSource& input);

    // Polls for quit first, returning false immediately without advancing/drawing if so; otherwise
    // advances the clock by elapsed_ms, applies one pending click, and redraws only if movement or
    // selection changed or an animation frame advanced (input is still polled every tick regardless).
    // Returns whether the game is still running.
    bool tick(int elapsed_ms);

private:
    Controller& controller_;
    IRenderer& renderer_;
    IInputSource& input_;

    bool first_tick_ = true;
    std::optional<Position> last_selected_;
    RenderSnapshot cached_snapshot_; // last-drawn piece list; reused as-is on animation-only or idle ticks
};
