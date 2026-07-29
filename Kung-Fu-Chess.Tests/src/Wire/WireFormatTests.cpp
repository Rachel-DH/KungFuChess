#include "ThirdParty/doctest.h"

#include "wire/WireFormat.h"

TEST_SUITE("WireFormat::Position") {

TEST_CASE("to_json of the board's minimum corner") {
    nlohmann::json j = Position{ 0, 0 };
    CHECK(j["x"] == 0);
    CHECK(j["y"] == 0);
}

TEST_CASE("to_json of the board's maximum corner") {
    nlohmann::json j = Position{ 7, 7 };
    CHECK(j["x"] == 7);
    CHECK(j["y"] == 7);
}

TEST_CASE("to_json accepts an off-board position unchanged") {
    nlohmann::json j = Position{ -1, -1 };
    CHECK(j["x"] == -1);
    CHECK(j["y"] == -1);
}

TEST_CASE("from_json of a plain object produces the matching position") {
    nlohmann::json j = { { "x", 3 }, { "y", 4 } };
    Position pos = j.get<Position>();
    CHECK(pos.x == 3);
    CHECK(pos.y == 4);
}

TEST_CASE("from_json throws when a field is missing") {
    nlohmann::json j = { { "x", 3 } };
    CHECK_THROWS_AS(j.get<Position>(), nlohmann::json::exception);
}

TEST_CASE("from_json throws when a field has the wrong type") {
    nlohmann::json j = { { "x", "abc" }, { "y", 4 } };
    CHECK_THROWS_AS(j.get<Position>(), nlohmann::json::exception);
}

TEST_CASE("round-trip symmetry holds for two representative positions") {
    CHECK(nlohmann::json(Position{ 0, 0 }).get<Position>() == Position{ 0, 0 });
    CHECK(nlohmann::json(Position{ 7, 7 }).get<Position>() == Position{ 7, 7 });
}

}
