#include "RenderLoop.h"

RenderLoop::RenderLoop(Controller& controller, IRenderer& renderer, IInputSource& input)
    : controller_(controller), renderer_(renderer), input_(input) {
}

bool RenderLoop::tick(int elapsed_ms) {
    bool quit = input_.poll_quit();
    if (quit) {
        return false;
    }

    controller_.wait(elapsed_ms);

    std::optional<std::pair<int, int>> click = input_.poll_click();
    if (click) {
        controller_.click(click->first, click->second);
    }

    renderer_.draw(controller_.piece_display_states());
    return !controller_.game_over();
}
