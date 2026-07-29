#include "ThirdParty/doctest.h"

#include <sstream>

#include "control/Controller.h"
#include "model/GameEngine.h"
#include "input/Parser.h"

namespace {

Board make_board() {
    return Parser::parse_board({
        "bR bN .",
        ".  .  .",
        "wR .  wN",
    });
}

std::string board_of(const Controller& controller) {
    std::ostringstream oss;
    controller.print(oss);
    return oss.str();
}

} // namespace

TEST_SUITE("Controller") {

// ---- out-of-bounds cells ---------------------------------------------------

TEST_CASE("clicking a cell outside the board is ignored") {
    SUBCASE("negative x") {
        Controller controller(make_board());
        controller.click(Position{ -1, 1 });
        CHECK_FALSE(controller.has_selection());
    }
    SUBCASE("negative y") {
        Controller controller(make_board());
        controller.click(Position{ 1, -1 });
        CHECK_FALSE(controller.has_selection());
    }
    SUBCASE("x past the last column") {
        Controller controller(make_board());
        controller.click(Position{ 3, 1 }); // board is 3 columns wide (0..2)
        CHECK_FALSE(controller.has_selection());
    }
    SUBCASE("y past the last row") {
        Controller controller(make_board());
        controller.click(Position{ 1, 3 }); // board is 3 rows tall (0..2)
        CHECK_FALSE(controller.has_selection());
    }
}

TEST_CASE("clicking a cell outside the board cancels an active selection") {
    Controller controller(make_board());
    controller.click(Position{ 0, 0 }); // select bR at (0,0)
    REQUIRE(controller.has_selection());

    controller.click(Position{ -1, 0 }); // outside the board
    CHECK_FALSE(controller.has_selection());
}

TEST_CASE("clicking on a degenerate 0x0 board is always ignored") {
    Controller controller(Parser::parse_board({}));
    controller.click(Position{ 0, 0 });
    CHECK_FALSE(controller.has_selection());
}

TEST_CASE("jumping a cell outside the board is silently ignored") {
    Controller controller(Parser::parse_board({ "wR . ." }));
    controller.jump(Position{ -1, -1 });
    CHECK(board_of(controller) == "wR . .\n");
}

// ---- deselect() -------------------------------------------------------------

TEST_CASE("deselect clears an active selection") {
    Controller controller(make_board());
    controller.click(Position{ 0, 0 });
    REQUIRE(controller.has_selection());

    controller.deselect();
    CHECK_FALSE(controller.has_selection());
}

TEST_CASE("deselect with no active selection is a no-op") {
    Controller controller(make_board());
    controller.deselect();
    CHECK_FALSE(controller.has_selection());
}

// ---- selecting with nothing currently selected -----------------------------

TEST_CASE("clicking an empty cell with no selection is ignored") {
    Controller controller(make_board());
    controller.click(Position{ 2, 0 }); // empty
    controller.click(Position{ 1, 1 }); // empty
    CHECK_FALSE(controller.has_selection());
}

TEST_CASE("clicking a piece with no selection selects it") {
    Controller controller(make_board());
    controller.click(Position{ 0, 0 }); // bR
    REQUIRE(controller.has_selection());
    CHECK(controller.selected()->x == 0);
    CHECK(controller.selected()->y == 0);
}

// ---- replacing the selection ----------------------------------------------

TEST_CASE("clicking another friendly piece replaces the selection") {
    Controller controller(make_board());
    controller.click(Position{ 0, 0 }); // select bR at (0,0)
    controller.click(Position{ 1, 0 }); // bN at (1,0) is also black
    REQUIRE(controller.has_selection());
    CHECK(controller.selected()->x == 1);
    CHECK(controller.selected()->y == 0);
    CHECK(board_of(controller) == Parser::board_to_string(make_board()) + "\n");
}

TEST_CASE("re-clicking the same selected piece keeps it selected") {
    Controller controller(make_board());
    controller.click(Position{ 0, 0 });
    controller.click(Position{ 0, 0 });
    REQUIRE(controller.has_selection());
    CHECK(controller.selected()->x == 0);
    CHECK(controller.selected()->y == 0);
}

// ---- move requests (settle after the travel time elapses) ------------------

TEST_CASE("clicking an empty cell while a piece is selected eventually moves it there") {
    Controller controller(make_board());
    controller.click(Position{ 0, 0 }); // select bR at (0,0)
    controller.click(Position{ 0, 1 }); // empty; straight down is a legal rook move

    CHECK_FALSE(controller.has_selection());
    controller.wait(GameEngine::DEFAULT_MOVE_MS_PER_CELL); // 1 cell of travel time
    CHECK(board_of(controller) == ". bN .\nbR . .\nwR . wN\n");
}

TEST_CASE("clicking an enemy piece while a piece is selected eventually captures it") {
    Controller controller(make_board());
    controller.click(Position{ 0, 0 }); // select bR at (0,0)
    controller.click(Position{ 0, 2 }); // wR at (0,2) is white; straight down the column

    CHECK_FALSE(controller.has_selection());
    controller.wait(2 * GameEngine::DEFAULT_MOVE_MS_PER_CELL); // 2 cells of travel time
    CHECK(board_of(controller) == ". bN .\n. . .\nbR . wN\n");
}

TEST_CASE("the vacated source cell is empty once the move has arrived") {
    Controller controller(make_board());
    controller.click(Position{ 0, 0 }); // select bR at (0,0)
    controller.click(Position{ 0, 1 }); // move down to (0,1)
    controller.wait(GameEngine::DEFAULT_MOVE_MS_PER_CELL);

    controller.click(Position{ 0, 0 }); // (0,0) is now empty; no selection to move
    CHECK_FALSE(controller.has_selection());
}

TEST_CASE("a cell with a move in flight cannot be reselected") {
    Controller controller(make_board());
    controller.click(Position{ 0, 0 }); // select bR at (0,0)
    controller.click(Position{ 0, 1 }); // move down to (0,1); still in flight

    controller.click(Position{ 0, 0 }); // (0,0) still shows bR, but it's mid-move
    CHECK_FALSE(controller.has_selection());
}

TEST_CASE("consecutive moves chain correctly once each one arrives") {
    Controller controller(make_board());
    controller.click(Position{ 0, 0 }); // select bR at (0,0)
    controller.click(Position{ 0, 1 }); // move bR down to (0,1)
    controller.wait(GameEngine::DEFAULT_MOVE_MS_PER_CELL);

    controller.click(Position{ 0, 1 }); // re-select the piece that is now at (0,1)
    REQUIRE(controller.has_selection());
    CHECK(controller.selected()->x == 0);
    CHECK(controller.selected()->y == 1);

    controller.click(Position{ 0, 2 }); // move down to (0,2), capturing wR
    controller.wait(GameEngine::DEFAULT_MOVE_MS_PER_CELL);
    CHECK_FALSE(controller.has_selection());
    CHECK(board_of(controller) == ". bN .\n. . .\nbR . wN\n");
}

// ---- movement over time -----------------------------------------------------

TEST_CASE("the board still shows the piece at its original cell before arrival") {
    Controller controller(make_board());
    controller.click(Position{ 0, 0 }); // select bR at (0,0)
    controller.click(Position{ 0, 1 }); // 1 cell of travel time is needed to arrive

    controller.wait(GameEngine::DEFAULT_MOVE_MS_PER_CELL - 1); // one millisecond short
    CHECK(board_of(controller) == Parser::board_to_string(make_board()) + "\n");
}

TEST_CASE("the piece appears at the destination once enough time has passed") {
    Controller controller(make_board());
    controller.click(Position{ 0, 0 }); // select bR at (0,0)
    controller.click(Position{ 0, 1 }); // 1 cell of travel time is needed to arrive

    controller.wait(GameEngine::DEFAULT_MOVE_MS_PER_CELL);
    CHECK(board_of(controller) == ". bN .\nbR . .\nwR . wN\n");
}

// ---- a moving piece cannot be redirected; no cooldown once it arrives ------

TEST_CASE("a piece already moving cannot be redirected to a new destination") {
    Controller controller(make_board());
    controller.click(Position{ 0, 0 }); // select bR at (0,0)
    controller.click(Position{ 0, 1 }); // move down to (0,1); 1 cell of travel time

    // Attempt to redirect it mid-route by reselecting its (still visible) origin cell; this fails, so there is no selection left to redirect.
    controller.click(Position{ 0, 0 });
    CHECK_FALSE(controller.has_selection());

    // Once the original move arrives, the piece is at its first destination only; the redirect attempt had no effect.
    controller.wait(GameEngine::DEFAULT_MOVE_MS_PER_CELL);
    CHECK(board_of(controller) == ". bN .\nbR . .\nwR . wN\n");
}

TEST_CASE("a piece can be selected and moved again immediately after arriving, with no cooldown") {
    Controller controller(make_board());
    controller.click(Position{ 0, 0 }); // select bR at (0,0)
    controller.click(Position{ 0, 1 }); // move down to (0,1)
    controller.wait(GameEngine::DEFAULT_MOVE_MS_PER_CELL); // arrives; no extra wait afterward

    controller.click(Position{ 0, 1 }); // select the just-arrived piece right away
    REQUIRE(controller.has_selection());
    CHECK(controller.selected()->x == 0);
    CHECK(controller.selected()->y == 1);

    controller.click(Position{ 0, 2 }); // immediately move it again, down to (0,2), capturing wR
    CHECK_FALSE(controller.has_selection());
    controller.wait(GameEngine::DEFAULT_MOVE_MS_PER_CELL);
    CHECK(board_of(controller) == ". bN .\n. . .\nbR . wN\n");
}

// ---- moves that don't match the piece's shape are ignored -------------------

TEST_CASE("a move that doesn't match the selected piece's shape is ignored") {
    Controller controller(make_board());
    controller.click(Position{ 0, 0 }); // select bR at (0,0)
    controller.click(Position{ 1, 1 }); // diagonal move; illegal for a rook

    REQUIRE(controller.has_selection());
    CHECK(controller.selected()->x == 0);
    CHECK(controller.selected()->y == 0);
    CHECK(board_of(controller) == Parser::board_to_string(make_board()) + "\n");
}

TEST_CASE("a blocked straight move is ignored") {
    Controller controller(Parser::parse_board({
        "bR .",
        "bN .",
        ".  .",
    }));
    controller.click(Position{ 0, 0 }); // select bR at (0,0)
    controller.click(Position{ 0, 2 }); // past bN, which blocks the column at (0,1)

    REQUIRE(controller.has_selection());
    CHECK(controller.selected()->x == 0);
    CHECK(controller.selected()->y == 0);
}

// ---- colliding moves on a shared route --------------------------------------

TEST_CASE("when two pieces attempt to swap places along the same route, whichever moved first wins") {
    SUBCASE("white moves first") {
        Controller controller(Parser::parse_board({ "wR . . bR" }));
        controller.click(Position{ 0, 0 }); // select wR at (0,0)
        controller.click(Position{ 3, 0 }); // move wR across to (3,0), capturing bR; 3 cells of travel time
        controller.click(Position{ 3, 0 }); // select bR at (3,0); it hasn't moved yet
        controller.click(Position{ 0, 0 }); // attempt to move bR back to (0,0); collides with wR's route

        controller.wait(3 * GameEngine::DEFAULT_MOVE_MS_PER_CELL);
        CHECK(board_of(controller) == ". . . wR\n");
    }

    SUBCASE("black moves first") {
        Controller controller(Parser::parse_board({ "wR . . bR" }));
        controller.click(Position{ 3, 0 }); // select bR at (3,0)
        controller.click(Position{ 0, 0 }); // move bR across to (0,0), capturing wR; 3 cells of travel time
        controller.click(Position{ 0, 0 }); // select wR at (0,0); it hasn't moved yet
        controller.click(Position{ 3, 0 }); // attempt to move wR back to (3,0); collides with bR's route

        controller.wait(3 * GameEngine::DEFAULT_MOVE_MS_PER_CELL);
        CHECK(board_of(controller) == "bR . . .\n");
    }
}

TEST_CASE("a move rejected for colliding with another move's route keeps the current selection") {
    Controller controller(Parser::parse_board({ "wR . . bR" }));
    controller.click(Position{ 0, 0 }); // select wR at (0,0)
    controller.click(Position{ 3, 0 }); // move wR across to (3,0); still in flight
    controller.click(Position{ 3, 0 }); // select bR at (3,0)
    controller.click(Position{ 0, 0 }); // attempt to move bR back to (0,0); collides, rejected

    REQUIRE(controller.has_selection());
    CHECK(controller.selected()->x == 3);
    CHECK(controller.selected()->y == 0);
}

TEST_CASE("a move onto another route's cell is rejected for its own illegality, not treated as a collision") {
    Controller controller(Parser::parse_board({
        ".  .  .  .",
        "wQ .  .  bK",
        ".  .  bP .",
        ".  .  .  .",
    }));
    controller.click(Position{ 0, 1 }); // select wQ at (0,1)
    controller.click(Position{ 3, 1 }); // move wQ across to (3,1), capturing bK; 3 cells of travel time
    controller.wait(200);

    controller.click(Position{ 2, 2 }); // select bP at (2,2)
    controller.click(Position{ 2, 1 }); // (2,1) is on wQ's route, but this move is illegal anyway: backward for black

    controller.wait(3000);
    CHECK(board_of(controller) == ". . . .\n. . . wQ\n. . bP .\n. . . .\n");
}

// ---- friendly-piece landing ---------------------------------------------------

TEST_CASE("a knight cannot land on a friendly piece, even though the move shape is legal") {
    Controller controller(Parser::parse_board({
        ".  wP .",
        ".  .  .",
        "wN .  .",
    }));
    controller.click(Position{ 0, 2 }); // select wN at (0,2)
    controller.click(Position{ 1, 0 }); // attempt an L-shaped move onto wP at (1,0); same color

    controller.wait(GameEngine::DEFAULT_MOVE_MS_PER_CELL);
    CHECK(board_of(controller) == ". wP .\n. . .\nwN . .\n");
}

// ---- premoves that never became legal ------------------------------------------

TEST_CASE("a click on an unrelated empty cell after a failed redirect schedules nothing") {
    Controller controller(Parser::parse_board({ "wR . ." }));
    controller.click(Position{ 0, 0 }); // select wR at (0,0)
    controller.click(Position{ 1, 0 }); // move to (1,0); 1 cell of travel time
    controller.click(Position{ 0, 0 }); // attempt to redirect; fails, nothing selected
    controller.click(Position{ 2, 0 }); // empty and nothing is selected; ignored

    controller.wait(2 * GameEngine::DEFAULT_MOVE_MS_PER_CELL);
    CHECK(board_of(controller) == ". wR .\n");
}

// ---- jumping and the selection ------------------------------------------------

TEST_CASE("selecting then jumping the same piece clears the selection") {
    Controller controller(Parser::parse_board({ "wR . ." }));
    controller.click(Position{ 0, 0 }); // select wR at (0,0)
    REQUIRE(controller.has_selection());

    controller.jump(Position{ 0, 0 }); // jumping the selected piece drops the selection
    CHECK_FALSE(controller.has_selection());

    controller.click(Position{ 2, 0 }); // nothing selected, empty cell -> no move scheduled
    controller.wait(GameEngine::JUMP_DURATION_MS);
    CHECK(board_of(controller) == "wR . .\n");
}

// ---- once the game is over -----------------------------------------------------

TEST_CASE("once the game is over, further clicks are ignored") {
    Controller controller(Parser::parse_board({ "wR . bK", "wN . ." }));
    controller.click(Position{ 0, 0 }); // select wR at (0,0)
    controller.click(Position{ 2, 0 }); // move across to (2,0), capturing bK
    controller.wait(2 * GameEngine::DEFAULT_MOVE_MS_PER_CELL);
    REQUIRE(controller.game_over());

    controller.click(Position{ 0, 1 }); // attempt to select wN at (0,1); should be ignored
    CHECK_FALSE(controller.has_selection());
    CHECK(board_of(controller) == ". . wR\nwN . .\n");
}

TEST_CASE("once the game is over, a click outside the board still clears any stale selection") {
    Controller controller(Parser::parse_board({ "wR . bK", "wN . ." }));
    controller.click(Position{ 0, 0 }); // select wR at (0,0)
    controller.click(Position{ 2, 0 }); // move across to (2,0), capturing bK
    controller.wait(2 * GameEngine::DEFAULT_MOVE_MS_PER_CELL);
    REQUIRE(controller.game_over());

    controller.click(Position{ 0, 1 }); // attempt to select wN; ignored, no selection created
    controller.click(Position{ -1, 0 }); // outside the board
    CHECK_FALSE(controller.has_selection());
}

}
