#include "renderer.h"

#include <utility>

Renderer::Renderer(std::string board_path, std::string pieces_dir,
                    std::pair<int, int> board_size, int cell_width, int cell_height)
    : board_path_(std::move(board_path)),
      board_size_(board_size),
      cell_width_(cell_width),
      cell_height_(cell_height),
      sprite_manager_(std::move(pieces_dir), cell_width, cell_height)
{
}

void Renderer::draw(const std::vector<PieceDisplayState>& states, int elapsed_ms) {
    Img board;
    board.read(board_path_, board_size_, false);

    for (const auto& state : states) {
        Img& piece = sprite_manager_.sprite_for(state.id, state.type, state.color, state.phase, elapsed_ms);
        piece.draw_on(board, state.position.x * cell_width_, state.position.y * cell_height_);
    }
    cv::imshow("Kung Fu Chess", board.get_mat());
}