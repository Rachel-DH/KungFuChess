#pragma once

#include <string>
#include <vector>

#include "GameEngine.h"
#include "Types.h"

// Pixels - UI-facing only. Built by RenderLoop via BoardMapper; this is what Renderer receives.
struct PixelPosition {
    int x;
    int y;
};

struct PieceRenderState {
    int id;
    PieceType type;
    Color color;
    PiecePhase phase;
    PixelPosition pixel_position;  // already converted - Renderer doesn't convert it itself
    bool is_selected = false;      // already computed - Renderer doesn't decide, only draws per it
};

struct RenderSnapshot {
    std::vector<PieceRenderState> pieces;
    int score = 0;
    std::string status_text;
};
