#pragma once

#include <vector>

#include "GameEngine.h"

// Abstract drawing surface for a GameEngine snapshot, so main() can drive
// rendering without depending on any concrete (OpenCV-backed) implementation.
class IRenderer {
public:
    virtual ~IRenderer() = default;

    virtual void draw(const std::vector<PieceDisplayState>& states, int elapsed_ms) = 0;
};
