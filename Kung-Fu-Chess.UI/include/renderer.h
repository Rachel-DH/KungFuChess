#pragma once

#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "GameEngine.h"
#include "IRenderer.h"
#include "img.h"

// Draws a GameEngine snapshot to a window. Loop-free: an external caller
// owns the frame loop and calls draw() once per tick, the same split
// ProtocolIO (not Controller) uses for the engine's own read loop.
class Renderer : public IRenderer {
public:
    Renderer(std::string board_path, std::string pieces_dir,
             std::pair<int, int> board_size, int cell_width, int cell_height);

    // Draws the board, then every piece in `states` on top of it, and shows
    // the composed frame. Does not poll input or pace frames — that's the
    // caller's job. `elapsed_ms` is the time advanced this tick; not yet
    // consumed here (a later step drives per-piece animation from it).
    void draw(const std::vector<PieceDisplayState>& states, int elapsed_ms) override;

private:
    // Returns the idle sprite for (type, color), loading and caching it on
    // first use so repeated draw() calls don't re-read the same file.
    Img& piece_image(PieceType type, Color color);

    std::string board_path_;
    std::string pieces_dir_;
    std::pair<int, int> board_size_;
    int cell_width_;
    int cell_height_;

    std::unordered_map<std::string, Img> piece_images_;
};
