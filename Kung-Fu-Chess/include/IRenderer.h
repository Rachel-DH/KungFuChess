#pragma once

#include "RenderSnapshot.h"

// Abstract drawing surface for a GameEngine snapshot, so main() can drive rendering without depending on any concrete (OpenCV-backed) implementation.
class IRenderer {
public:
    virtual ~IRenderer() = default;

    virtual void draw(const RenderSnapshot& snapshot, int elapsed_ms) = 0;

    // Advances animation state by elapsed_ms, independent of drawing. Returns true if
    // any animation's current frame changed, so a caller can tell whether a subsequent
    // draw() would actually look different.
    virtual bool advance_animations(int elapsed_ms) = 0;
};
