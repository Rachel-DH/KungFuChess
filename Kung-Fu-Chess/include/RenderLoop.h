#pragma once

#include "Controller.h"
#include "IInputSource.h"
#include "IRenderer.h"

// Adapter: drives one windowed frame — advance the clock, apply a pending
// click, redraw — the windowed counterpart to CommandProcessor's per-line
// text protocol. Owns none of Controller/IRenderer/IInputSource; main()
// constructs them once and keeps this loop running until it returns false.
class RenderLoop {
public:
    RenderLoop(Controller& controller, IRenderer& renderer, IInputSource& input);

    // Polls for quit first: if the user asked to quit, returns false
    // immediately without advancing the clock, applying a click, or
    // drawing this call. Otherwise advances the clock by elapsed_ms,
    // applies at most one pending click, redraws, and returns whether the
    // game is still running.
    bool tick(int elapsed_ms);

private:
    Controller& controller_;
    IRenderer& renderer_;
    IInputSource& input_;
};
