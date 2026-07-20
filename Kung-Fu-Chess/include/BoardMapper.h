#pragma once

#include <optional>

#include "Position.h"
#include "RenderSnapshot.h"

// Adapter: converts between pixel coordinates and logical board cells; knows nothing about chess, pieces, or selection state.
class BoardMapper {
public:
    // Returns the cell at (pixel_x, pixel_y), or nullopt if outside the board.
    static std::optional<Position> pixel_to_cell(int pixel_x, int pixel_y, int board_width, int board_height);

    // Returns the top-left pixel of pos's cell. No bounds checking - an off-board pos still computes arithmetically.
    static PixelPosition cell_to_pixel(Position pos, int cell_width, int cell_height);
};
