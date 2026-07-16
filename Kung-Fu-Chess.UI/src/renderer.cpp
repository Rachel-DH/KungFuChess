#include "renderer.h"

#include <stdexcept>
#include <utility>

namespace {

char type_letter(PieceType type) {
    switch (type) {
        case PieceType::K: return 'K';
        case PieceType::Q: return 'Q';
        case PieceType::R: return 'R';
        case PieceType::B: return 'B';
        case PieceType::N: return 'N';
        case PieceType::P: return 'P';
    }
    throw std::invalid_argument("Unknown PieceType");
}

// Sprite folders are named "<type letter><color letter>", e.g. "BB" for a
// black bishop, matching ext_src/pieces2's layout.
std::string piece_code(PieceType type, Color color) {
    char color_letter = (color == Color::w) ? 'W' : 'B';
    return std::string(1, type_letter(type)) + color_letter;
}

}  // namespace

Renderer::Renderer(std::string board_path, std::string pieces_dir,
                    std::pair<int, int> board_size, int cell_width, int cell_height)
    : board_path_(std::move(board_path)),
      pieces_dir_(std::move(pieces_dir)),
      board_size_(board_size),
      cell_width_(cell_width),
      cell_height_(cell_height) {
}

Img& Renderer::piece_image(PieceType type, Color color) {
    std::string code = piece_code(type, color);
    auto cached = piece_images_.find(code);
    if (cached != piece_images_.end()) {
        return cached->second;
    }

    Img sprite;
    sprite.read(pieces_dir_ + "/" + code + "/states/idle/sprites/1.png",
                { cell_width_, cell_height_ }, true);
    return piece_images_.emplace(code, std::move(sprite)).first->second;
}

void Renderer::draw(const std::vector<PieceDisplayState>& states) {
    Img board;
    board.read(board_path_, board_size_, false);

    for (const auto& state : states) {
        Img& piece = piece_image(state.type, state.color);
        piece.draw_on(board, state.position.x * cell_width_, state.position.y * cell_height_);
    }

    cv::imshow("Kung Fu Chess", board.get_mat());
}
