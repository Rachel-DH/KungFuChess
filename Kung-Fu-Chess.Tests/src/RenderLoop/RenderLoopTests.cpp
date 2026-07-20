#include "ThirdParty/doctest.h"

#include <optional>
#include <utility>
#include <vector>

#include "BoardMapper.h"
#include "Constants.h"
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
    RenderSnapshot last_snapshot;

    void draw(const RenderSnapshot& snapshot, int elapsed_ms) override {
        ++draw_count;
        last_elapsed_ms = elapsed_ms;
        last_snapshot = snapshot;
    }
};

// Finds the render state at `cell`'s pixel position, if any occupies it.
std::optional<PieceRenderState> state_at(const std::vector<PieceRenderState>& pieces, Position cell) {
    PixelPosition pixel = BoardMapper::cell_to_pixel(cell, constants::kCellSizePx, constants::kCellSizePx);
    for (const auto& piece : pieces) {
        if (piece.pixel_position.x == pixel.x && piece.pixel_position.y == pixel.y) {
            return piece;
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

    std::optional<PieceRenderState> arrived = state_at(renderer.last_snapshot.pieces, Position{ 1, 0 });
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
    controller.click(constants::kCellSizePx / 2, constants::kCellSizePx / 2);                             // select wR at (0,0)
    controller.click(2 * constants::kCellSizePx + constants::kCellSizePx / 2, constants::kCellSizePx / 2); // move across to (2,0), capturing bK; 2 cells of travel time

    FakeInputSource input;
    FakeRenderer renderer;
    RenderLoop loop(controller, renderer, input);

    CHECK_FALSE(loop.tick(2 * GameEngine::kDefaultMoveMsPerCell));
    CHECK(renderer.draw_count == 1);
}

TEST_CASE("a piece at logical (0,0) is drawn at pixel (0,0)") {
    Controller controller(Parser::parse_board({ "wR . ." }));
    FakeInputSource input;
    FakeRenderer renderer;
    RenderLoop loop(controller, renderer, input);

    loop.tick(0);

    std::optional<PieceRenderState> wr = state_at(renderer.last_snapshot.pieces, Position{ 0, 0 });
    REQUIRE(wr.has_value());
    CHECK(wr->pixel_position.x == 0);
    CHECK(wr->pixel_position.y == 0);
}

TEST_CASE("a piece at logical (1,0) is drawn at pixel (kCellSizePx,0)") {
    Controller controller(Parser::parse_board({ ". wR ." }));
    FakeInputSource input;
    FakeRenderer renderer;
    RenderLoop loop(controller, renderer, input);

    loop.tick(0);

    std::optional<PieceRenderState> wr = state_at(renderer.last_snapshot.pieces, Position{ 1, 0 });
    REQUIRE(wr.has_value());
    CHECK(wr->pixel_position.x == constants::kCellSizePx);
    CHECK(wr->pixel_position.y == 0);
}

TEST_CASE("the selected piece's is_selected is true while Controller has an active selection") {
    Controller controller(Parser::parse_board({ "wR . ." }));
    controller.click(50, 50); // select wR at (0,0)
    REQUIRE(controller.has_selection());

    FakeInputSource input;
    FakeRenderer renderer;
    RenderLoop loop(controller, renderer, input);

    loop.tick(0);

    std::optional<PieceRenderState> wr = state_at(renderer.last_snapshot.pieces, Position{ 0, 0 });
    REQUIRE(wr.has_value());
    CHECK(wr->is_selected);
}

TEST_CASE("after a click selects one piece, only that piece's is_selected is true and every other piece's is false") {
    Controller controller(Parser::parse_board({ "wR . bK" }));
    controller.click(50, 50); // select wR at (0,0)
    REQUIRE(controller.has_selection());

    FakeInputSource input;
    FakeRenderer renderer;
    RenderLoop loop(controller, renderer, input);

    loop.tick(0);

    REQUIRE(renderer.last_snapshot.pieces.size() == 2);
    for (const PieceRenderState& piece : renderer.last_snapshot.pieces) {
        bool is_selected_piece = piece.pixel_position.x == 0 && piece.pixel_position.y == 0;
        CHECK(piece.is_selected == is_selected_piece);
    }
}

}
