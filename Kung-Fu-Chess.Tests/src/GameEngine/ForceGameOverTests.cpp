#include "ThirdParty/doctest.h"

#include "model/GameEngine.h"
#include "input/Parser.h"

TEST_SUITE("GameEngine::force_game_over") {

TEST_CASE("forcing game over ends the game even with no king captured") {
    GameEngine engine(Parser::parse_board({ "wK bK" }));
    CHECK_FALSE(engine.game_over());

    engine.force_game_over();
    CHECK(engine.game_over());
}

TEST_CASE("once forced over, further move requests are ignored") {
    GameEngine engine(Parser::parse_board({ "wR . bK" }));
    engine.force_game_over();

    CHECK_FALSE(engine.request_move(Position{ 0, 0 }, Position{ 2, 0 }));
}

}
