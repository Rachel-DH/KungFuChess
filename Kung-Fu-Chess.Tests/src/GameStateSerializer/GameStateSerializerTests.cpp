#include "ThirdParty/doctest.h"

#include "GameStateSerializer.h"
#include "ThirdParty/nlohmann/json.hpp"
#include "input/Parser.h"

using nlohmann::json;

TEST_SUITE("GameStateSerializer::to_json") {

TEST_CASE("serializes every piece's position, type, color, and phase") {
    GameEngine engine(Parser::parse_board({ "wK . bR" }));
    json parsed = json::parse(GameStateSerializer::to_json(engine, {}));

    CHECK(parsed["type"] == "game_state");
    CHECK(parsed["game_over"] == false);
    REQUIRE(parsed["pieces"].size() == 2);

    bool found_king = false;
    bool found_rook = false;
    for (const auto& piece : parsed["pieces"]) {
        if (piece["type"] == "K") {
            found_king = true;
            CHECK(piece["color"] == "w");
            CHECK(piece["x"] == 0);
            CHECK(piece["y"] == 0);
            CHECK(piece["phase"] == "idle");
        }
        if (piece["type"] == "R") {
            found_rook = true;
            CHECK(piece["color"] == "b");
        }
    }
    CHECK(found_king);
    CHECK(found_rook);
}

TEST_CASE("reflects game_over once the game has ended") {
    GameEngine engine(Parser::parse_board({ "wR . bK" }));
    engine.request_move(Position{ 0, 0 }, Position{ 2, 0 });
    engine.wait(2 * GameEngine::DEFAULT_MOVE_MS_PER_CELL);
    REQUIRE(engine.game_over());

    json parsed = json::parse(GameStateSerializer::to_json(engine, {}));
    CHECK(parsed["game_over"] == true);
}

TEST_CASE("serializes a capture event with its position and captured piece") {
    GameEngine engine(Parser::parse_board({ "wR . bR" }));
    std::vector<Event> events{ CaptureEvent{ Position{ 2, 0 }, PieceType::R, Color::b } };

    json parsed = json::parse(GameStateSerializer::to_json(engine, events));
    REQUIRE(parsed["events"].size() == 1);
    CHECK(parsed["events"][0]["type"] == "capture");
    CHECK(parsed["events"][0]["at"]["x"] == 2);
    CHECK(parsed["events"][0]["captured_type"] == "R");
    CHECK(parsed["events"][0]["captured_color"] == "b");
}

TEST_CASE("serializes a game-over event with the losing color") {
    GameEngine engine(Parser::parse_board({ "wK bK" }));
    std::vector<Event> events{ CheckmateEvent{ Color::b } };

    json parsed = json::parse(GameStateSerializer::to_json(engine, events));
    REQUIRE(parsed["events"].size() == 1);
    CHECK(parsed["events"][0]["type"] == "game_over");
    CHECK(parsed["events"][0]["losing_color"] == "b");
}

TEST_CASE("serializes a disconnect event with the player id") {
    GameEngine engine(Parser::parse_board({ "wK bK" }));
    std::vector<Event> events{ PlayerDisconnectedEvent{ "alice" } };

    json parsed = json::parse(GameStateSerializer::to_json(engine, events));
    REQUIRE(parsed["events"].size() == 1);
    CHECK(parsed["events"][0]["type"] == "disconnect");
    CHECK(parsed["events"][0]["player_id"] == "alice");
}

TEST_CASE("an empty events list serializes to an empty array") {
    GameEngine engine(Parser::parse_board({ "wK bK" }));
    json parsed = json::parse(GameStateSerializer::to_json(engine, {}));
    CHECK(parsed["events"].empty());
}

}
