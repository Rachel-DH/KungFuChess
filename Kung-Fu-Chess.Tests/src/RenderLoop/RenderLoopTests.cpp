#include "ThirdParty/doctest.h"

#include <optional>
#include <utility>
#include <vector>

#include "Controller.h"
#include "GameEngine.h"
#include "IInputSource.h"
#include "IRenderer.h"
#include "Parser.h"
#include "RenderLoop.h"

namespace {

class FakeInputSource : public IInputSource {
public:
    bool quit = false;
    std::optional<std::pair<int, int>> pending_click;

    bool poll_quit() override { return quit; }

    std::optional<std::pair<int, int>> poll_click() override {
        std::optional<std::pair<int, int>> click = pending_click;
        pending_click.reset();
        return click;
    }
};

class FakeRenderer : public IRenderer {
public:
    int draw_count = 0;
    int last_elapsed_ms = -1;
    std::vector<PieceDisplayState> last_states;

    void draw(const std::vector<PieceDisplayState>& states, int elapsed_ms) override {
        ++draw_count;
        last_elapsed_ms = elapsed_ms;
        last_states = states;
    }
};

// Finds the display state at `cell`, if any occupies it.
std::optional<PieceDisplayState> state_at(const std::vector<PieceDisplayState>& states, Position cell) {
    for (const auto& state : states) {
        if (state.position.x == cell.x && state.position.y == cell.y) {
            return state;
        }
    }
    return std::nullopt;
}

} // namespace

TEST_SUITE("RenderLoop::tick") {

TEST_CASE("quitting stops the loop and does nothing else this tick") {
    Controller controller(Parser::parse_board({ "wR . ." }));
    FakeInputSource input;
    input.quit = true;
    input.pending_click = std::make_pair(50, 50); // (0,0), even though it's never applied
    FakeRenderer renderer;
    RenderLoop loop(controller, renderer, input);

    CHECK_FALSE(loop.tick(1000));
    CHECK(renderer.draw_count == 0);
}

TEST_CASE("no pending click still advances the clock and draws exactly once, with elapsed_ms forwarded exactly") {
    Controller controller(Parser::parse_board({ "wR . ." }));
    controller.click(50, 50);  // select wR at (0,0)
    controller.click(150, 50); // move to (1,0); 1 cell of travel time

    FakeInputSource input;
    FakeRenderer renderer;
    RenderLoop loop(controller, renderer, input);

    CHECK(loop.tick(GameEngine::kDefaultMoveMsPerCell));
    CHECK(renderer.draw_count == 1);
    CHECK(renderer.last_elapsed_ms == GameEngine::kDefaultMoveMsPerCell);

    std::optional<PieceDisplayState> arrived = state_at(renderer.last_states, Position{ 1, 0 });
    REQUIRE(arrived.has_value());
    CHECK(arrived->type == PieceType::R);
    CHECK(arrived->color == Color::w);
}

TEST_CASE("a pending click is forwarded to Controller::click, and draw still receives the tick's elapsed_ms") {
    Controller controller(Parser::parse_board({ "wR . ." }));
    FakeInputSource input;
    input.pending_click = std::make_pair(50, 50); // (0,0) = wR, selectable
    FakeRenderer renderer;
    RenderLoop loop(controller, renderer, input);

    CHECK(loop.tick(37));
    REQUIRE(controller.has_selection());
    CHECK(controller.selected()->x == 0);
    CHECK(controller.selected()->y == 0);
    CHECK(renderer.last_elapsed_ms == 37);
}

TEST_CASE("the clock advances before the click is applied, so a piece landing exactly this tick becomes selectable within the same tick") {
    Controller controller(Parser::parse_board({ "wR . ." }));
    controller.click(50, 50);  // select wR at (0,0)
    controller.click(150, 50); // move to (1,0); 1 cell of travel time

    FakeInputSource input;
    input.pending_click = std::make_pair(150, 50); // (1,0), the move's destination
    FakeRenderer renderer;
    RenderLoop loop(controller, renderer, input);

    loop.tick(GameEngine::kDefaultMoveMsPerCell);
    REQUIRE(controller.has_selection());
    CHECK(controller.selected()->x == 1);
    CHECK(controller.selected()->y == 0);
}

TEST_CASE("tick reports stop the instant wait ends the game, but still draws the final frame once") {
    Controller controller(Parser::parse_board({ "wR . bK" }));
    controller.click(50, 50);  // select wR at (0,0)
    controller.click(250, 50); // move across to (2,0), capturing bK; 2 cells of travel time

    FakeInputSource input;
    FakeRenderer renderer;
    RenderLoop loop(controller, renderer, input);

    CHECK_FALSE(loop.tick(2 * GameEngine::kDefaultMoveMsPerCell));
    CHECK(renderer.draw_count == 1);
}

}
