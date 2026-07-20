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

void Renderer::draw(const RenderSnapshot& snapshot, int elapsed_ms) {
    Img board;
    board.read(board_path_, board_size_, false);

    for (const auto& piece_state : snapshot.pieces) {
        Img& piece = sprite_manager_.sprite_for(piece_state.id, piece_state.type, piece_state.color, piece_state.phase, elapsed_ms);
        piece.draw_on(board, piece_state.pixel_position.x, piece_state.pixel_position.y);
        if (piece_state.is_selected) {
            board.draw_rect(piece_state.pixel_position.x, piece_state.pixel_position.y,
                             cell_width_, cell_height_, cv::Scalar(0, 0, 255, 255));
        }
    }
    cv::imshow("Kung Fu Chess", board.get_mat());
}