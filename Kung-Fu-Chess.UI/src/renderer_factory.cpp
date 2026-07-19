#include "RendererFactory.h"

#include "asset_path.h"
#include "renderer.h"

std::unique_ptr<IRenderer> make_renderer() {
    const auto exe_dir = asset_path::executable_dir();
    const auto board_path = asset_path::resolve(exe_dir, "../../../ext_src/board.png");
    const auto pieces_dir = asset_path::resolve(exe_dir, "../../../ext_src/pieces2");
    return std::make_unique<Renderer>(board_path.string(), pieces_dir.string(),
                                       std::pair<int, int>{640, 640}, 80, 80);
}
