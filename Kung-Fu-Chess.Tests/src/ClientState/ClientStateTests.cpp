#include "ThirdParty/doctest.h"

#include "ClientState.h"

TEST_SUITE("ClientState::apply_game_state_json") {

TEST_CASE("a fresh ClientState has no snapshot yet") {
    ClientState state;
    CHECK_FALSE(state.has_state());
}

TEST_CASE("parses every piece's position, type, color, and phase") {
    ClientState state;
    state.apply_game_state_json(
        R"({"type":"game_state","game_over":false,"events":[],"pieces":[)"
        R"({"id":0,"x":0,"y":6,"type":"P","color":"w","phase":"idle"}]})");

    REQUIRE(state.has_state());
    REQUIRE(state.pieces().size() == 1);
    const auto& piece = state.pieces().front();
    CHECK(piece.position.x == 0);
    CHECK(piece.position.y == 6);
    CHECK(piece.type == PieceType::P);
    CHECK(piece.color == Color::w);
    CHECK(piece.phase == PiecePhase::Idle);
    CHECK_FALSE(piece.is_moving);
    CHECK_FALSE(piece.is_airborne);
}

TEST_CASE("a moving piece is flagged is_moving and a jumping piece is_airborne") {
    ClientState state;
    state.apply_game_state_json(
        R"({"type":"game_state","game_over":false,"events":[],"pieces":[)"
        R"({"id":0,"x":0,"y":4,"type":"P","color":"w","phase":"move"},)"
        R"({"id":1,"x":3,"y":3,"type":"R","color":"b","phase":"jump"}]})");

    REQUIRE(state.pieces().size() == 2);
    CHECK(state.pieces()[0].is_moving);
    CHECK_FALSE(state.pieces()[0].is_airborne);
    CHECK(state.pieces()[1].is_airborne);
    CHECK_FALSE(state.pieces()[1].is_moving);
}

TEST_CASE("reflects the game_over flag") {
    ClientState state;
    state.apply_game_state_json(R"({"type":"game_state","game_over":true,"events":[],"pieces":[]})");
    CHECK(state.game_over());
}

TEST_CASE("a later snapshot fully replaces the previous one") {
    ClientState state;
    state.apply_game_state_json(
        R"({"type":"game_state","game_over":false,"events":[],"pieces":[)"
        R"({"id":0,"x":0,"y":6,"type":"P","color":"w","phase":"idle"}]})");
    state.apply_game_state_json(R"({"type":"game_state","game_over":false,"events":[],"pieces":[]})");

    CHECK(state.pieces().empty());
}

TEST_CASE("malformed JSON is ignored and the previous snapshot is kept") {
    ClientState state;
    state.apply_game_state_json(
        R"({"type":"game_state","game_over":false,"events":[],"pieces":[)"
        R"({"id":0,"x":0,"y":6,"type":"P","color":"w","phase":"idle"}]})");
    state.apply_game_state_json("not json");

    REQUIRE(state.pieces().size() == 1);
}

}

TEST_SUITE("ClientState::piece_at") {

TEST_CASE("finds the piece occupying a given cell") {
    ClientState state;
    state.apply_game_state_json(
        R"({"type":"game_state","game_over":false,"events":[],"pieces":[)"
        R"({"id":0,"x":2,"y":3,"type":"N","color":"b","phase":"idle"}]})");

    const PieceDisplayState* piece = state.piece_at(Position{ 2, 3 });
    REQUIRE(piece != nullptr);
    CHECK(piece->type == PieceType::N);
}

TEST_CASE("returns nullptr for an empty cell") {
    ClientState state;
    state.apply_game_state_json(R"({"type":"game_state","game_over":false,"events":[],"pieces":[]})");
    CHECK(state.piece_at(Position{ 0, 0 }) == nullptr);
}

}
