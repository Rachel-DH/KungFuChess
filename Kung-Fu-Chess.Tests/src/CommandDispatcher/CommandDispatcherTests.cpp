#include "ThirdParty/doctest.h"

#include "model/GameEngine.h"
#include "input/Parser.h"
#include "net/CommandDispatcher.h"

TEST_SUITE("CommandDispatcher::dispatch") {

TEST_CASE("a legal MoveCommand is accepted and forwarded to request_move") {
    GameEngine engine(Parser::parse_board({ "wR . ." }));
    CHECK(CommandDispatcher::dispatch(MoveCommand{ Position{ 0, 0 }, Position{ 2, 0 } }, engine));
}

TEST_CASE("an illegal MoveCommand is rejected and the board is unchanged") {
    GameEngine engine(Parser::parse_board({ "wN . ." }));
    CHECK_FALSE(CommandDispatcher::dispatch(MoveCommand{ Position{ 0, 0 }, Position{ 2, 0 } }, engine)); // not a knight shape
}

TEST_CASE("a JumpCommand on a selectable own piece is accepted and forwarded to request_jump") {
    GameEngine engine(Parser::parse_board({ "wR . ." }));
    CHECK(CommandDispatcher::dispatch(JumpCommand{ Position{ 0, 0 } }, engine));
}

TEST_CASE("a JumpCommand on an already-airborne piece is rejected") {
    GameEngine engine(Parser::parse_board({ "wR . ." }));
    REQUIRE(CommandDispatcher::dispatch(JumpCommand{ Position{ 0, 0 } }, engine));
    CHECK_FALSE(CommandDispatcher::dispatch(JumpCommand{ Position{ 0, 0 } }, engine));
}

TEST_CASE("any command is rejected once the game is over") {
    GameEngine engine(Parser::parse_board({ "wR . bK" }));
    REQUIRE(CommandDispatcher::dispatch(MoveCommand{ Position{ 0, 0 }, Position{ 2, 0 } }, engine));
    engine.wait(2 * GameEngine::DEFAULT_MOVE_MS_PER_CELL);
    REQUIRE(engine.game_over());

    SUBCASE("a MoveCommand") {
        CHECK_FALSE(CommandDispatcher::dispatch(MoveCommand{ Position{ 2, 0 }, Position{ 1, 0 } }, engine));
    }
    SUBCASE("a JumpCommand") {
        CHECK_FALSE(CommandDispatcher::dispatch(JumpCommand{ Position{ 2, 0 } }, engine));
    }
}

}
