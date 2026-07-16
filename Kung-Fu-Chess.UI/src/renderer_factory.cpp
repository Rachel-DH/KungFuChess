#include "RendererFactory.h"

#include "renderer.h"

std::unique_ptr<IRenderer> make_renderer() {
    return std::make_unique<Renderer>("../ext_src/board.png", "../ext_src/pieces2",
                                       std::pair<int, int>{640, 640}, 80, 80);
}
