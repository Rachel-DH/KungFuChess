#include "renderer.h"

#include <utility>
#include <vector>

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

    std::vector<int> live_ids;
    live_ids.reserve(snapshot.pieces.size());
    for (const auto& piece_state : snapshot.pieces) {
        Img& piece = sprite_manager_.sprite_for(piece_state.id, piece_state.type, piece_state.color, piece_state.phase);
        piece.draw_on(board, piece_state.pixel_position.x, piece_state.pixel_position.y);
        if (piece_state.is_selected) {
            board.draw_rect(piece_state.pixel_position.x, piece_state.pixel_position.y,
                             cell_width_, cell_height_, cv::Scalar(0, 0, 255, 255));
        }
        live_ids.push_back(piece_state.id);
    }
    cv::imshow("Kung Fu Chess", board.get_mat());

    // piece_id is derived from board position, so a piece that moved this tick leaves a
    // stale entry behind under its old id; drop anything not present in this snapshot.
    sprite_manager_.prune(live_ids);
}

bool Renderer::advance_animations(int elapsed_ms) {
    return sprite_manager_.advance_all(elapsed_ms);
}