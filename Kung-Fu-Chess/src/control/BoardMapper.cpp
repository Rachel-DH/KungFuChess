#include "control/BoardMapper.h"

std::optional<Position> BoardMapper::pixel_to_cell(int pixel_x, int pixel_y, int board_width, int board_height) {
    if (pixel_x < 0 || pixel_y < 0) {
        return std::nullopt;
    }

    int cell_x = pixel_x / CELL_SIZE_PX;
    int cell_y = pixel_y / CELL_SIZE_PX;

    if (cell_x >= board_width || cell_y >= board_height) {
        return std::nullopt;
    }

    return Position{ cell_x, cell_y };
}

PixelPosition BoardMapper::cell_to_pixel(Position pos, int cell_width, int cell_height) {
    return PixelPosition{ pos.x * cell_width, pos.y * cell_height };
}
