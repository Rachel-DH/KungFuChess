#include "ThirdParty/doctest.h"

#include "GameEngine.h"
#include "Parser.h"

TEST_SUITE("GameEngine::has_activity") {

TEST_CASE("a fresh engine with no pending moves or jumps has no activity") {
    GameEngine engine(Parser::parse_board({ "wK" }));
    CHECK_FALSE(engine.has_activity());
}

TEST_CASE("a piece with a pending move that hasn't arrived has activity") {
    GameEngine engine(Parser::parse_board({ "wK ." }));
    CHECK(engine.request_move(Position{ 0, 0 }, Position{ 1, 0 }));
    CHECK(engine.has_activity());
}

TEST_CASE("activity clears once the move settles") {
    GameEngine engine(Parser::parse_board({ "wK ." }));
    CHECK(engine.request_move(Position{ 0, 0 }, Position{ 1, 0 }));

    engine.wait(GameEngine::kDefaultMoveMsPerCell);
    CHECK_FALSE(engine.has_activity());
}

TEST_CASE("an airborne piece has activity") {
    GameEngine engine(Parser::parse_board({ "wK" }));
    CHECK(engine.request_jump(Position{ 0, 0 }));
    CHECK(engine.has_activity());
}

TEST_CASE("activity clears once the jump lands") {
    GameEngine engine(Parser::parse_board({ "wK" }));
    CHECK(engine.request_jump(Position{ 0, 0 }));

    engine.wait(GameEngine::kJumpDurationMs);
    CHECK_FALSE(engine.has_activity());
}

TEST_CASE("a pending move and an airborne piece together still report activity") {
    GameEngine engine(Parser::parse_board({ "wK . bR" }));
    CHECK(engine.request_move(Position{ 0, 0 }, Position{ 1, 0 })); // wK moves
    CHECK(engine.request_jump(Position{ 2, 0 }));                   // bR jumps
    CHECK(engine.has_activity());
}

}
