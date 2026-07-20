#pragma once

#include "RenderSnapshot.h"

// Abstract drawing surface for a GameEngine snapshot, so main() can drive rendering without depending on any concrete (OpenCV-backed) implementation.
class IRenderer {
public:
    virtual ~IRenderer() = default;

    virtual void draw(const RenderSnapshot& snapshot, int elapsed_ms) = 0;
};
