#pragma once

#include <string>
#include <utility>
#include <vector>

#include "GameEngine.h"
#include "IRenderer.h"
#include "img.h"
#include "sprite_manager.h"

// Draws a GameEngine snapshot to a window; loop-free, an external caller owns the frame loop and calls draw() once per tick, the same split ProtocolIO (not Controller) uses for the engine's own read loop.
class Renderer : public IRenderer {
public:
    Renderer(std::string board_path, std::string pieces_dir,
             std::pair<int, int> board_size, int cell_width, int cell_height);

    // Draws the board, then every piece in `states` on top of it, and shows the composed frame; does not poll input or pace frames (caller's job), and `elapsed_ms` isn't consumed here yet.
    void draw(const std::vector<PieceDisplayState>& states, int elapsed_ms) override;

private:
    std::string board_path_;
    std::pair<int, int> board_size_;
    int cell_width_;
    int cell_height_;

    SpriteManager sprite_manager_;
};
