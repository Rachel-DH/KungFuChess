#include "RenderLoop.h"

#include "BoardMapper.h"
#include "Constants.h"

namespace {

// Converts one logical display state into its pixel-space render state; is_selected is true iff this piece sits on Controller's currently selected cell.
PieceRenderState to_render_state(const PieceDisplayState& state, std::optional<Position> selected) {
    PixelPosition pixel_position =
        BoardMapper::cell_to_pixel(state.position, constants::kCellSizePx, constants::kCellSizePx);
    bool is_selected = selected.has_value() && *selected == state.position;
    return PieceRenderState{ state.id, state.type, state.color, state.phase, pixel_position, is_selected };
}

} // namespace

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

    std::optional<Position> selected = controller_.selected();
    RenderSnapshot snapshot;
    for (const PieceDisplayState& state : controller_.piece_display_states()) {
        snapshot.pieces.push_back(to_render_state(state, selected));
    }

    renderer_.draw(snapshot, elapsed_ms);
    return !controller_.game_over();
}
