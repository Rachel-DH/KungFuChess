#include "RendererFactory.h"

#include "Constants.h"
#include "asset_path.h"
#include "renderer.h"

namespace {

    constexpr int kBoardCells = 8;
    constexpr int kBoardSizePx = kBoardCells * constants::kCellSizePx;

} // namespace

std::unique_ptr<IRenderer> make_renderer() {
    const auto exe_dir = asset_path::executable_dir();
    const auto board_path = asset_path::resolve(exe_dir, "../../../ext_src/board.png");
    const auto pieces_dir = asset_path::resolve(exe_dir, "../../../ext_src/pieces2");
    return std::make_unique<Renderer>(board_path.string(), pieces_dir.string(),
                                       std::pair<int, int>{kBoardSizePx, kBoardSizePx},
                                       constants::kCellSizePx, constants::kCellSizePx);
}
