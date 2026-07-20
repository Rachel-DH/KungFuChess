#pragma once

#include "Controller.h"
#include "IInputSource.h"
#include "IRenderer.h"

// Adapter: drives one windowed frame — advance the clock, apply a pending click, redraw — the windowed counterpart to CommandProcessor's per-line text protocol; owns none of Controller/IRenderer/IInputSource.
class RenderLoop {
public:
    RenderLoop(Controller& controller, IRenderer& renderer, IInputSource& input);

    // Polls for quit first, returning false immediately without advancing/drawing if so; otherwise advances the clock by elapsed_ms, applies one pending click, redraws, and returns whether the game is still running.
    bool tick(int elapsed_ms);

private:
    Controller& controller_;
    IRenderer& renderer_;
    IInputSource& input_;
};
