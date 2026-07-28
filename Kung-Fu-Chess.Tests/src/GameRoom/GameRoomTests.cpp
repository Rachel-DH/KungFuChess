#include "ThirdParty/doctest.h"

#include "GameRoom.h"

TEST_SUITE("GameRoom::join") {

TEST_CASE("the first player to join becomes the White opponent") {
    GameRoom room("room-1");
    JoinResult result = room.join("alice");
    CHECK(result.role == PlayerRole::Opponent);
    REQUIRE(result.color.has_value());
    CHECK(*result.color == Color::w);
}

TEST_CASE("the second player to join becomes the Black opponent") {
    GameRoom room("room-1");
    room.join("alice");
    JoinResult result = room.join("bob");
    CHECK(result.role == PlayerRole::Opponent);
    REQUIRE(result.color.has_value());
    CHECK(*result.color == Color::b);
}

TEST_CASE("a third player to join becomes a spectator") {
    GameRoom room("room-1");
    room.join("alice");
    room.join("bob");
    JoinResult result = room.join("carol");
    CHECK(result.role == PlayerRole::Spectator);
    CHECK_FALSE(result.color.has_value());
}

TEST_CASE("re-joining with the same id returns the same slot instead of a new one") {
    GameRoom room("room-1");
    room.join("alice");
    JoinResult result = room.join("alice");
    CHECK(result.role == PlayerRole::Opponent);
    CHECK(*result.color == Color::w);
}

}

TEST_SUITE("GameRoom::on_disconnect / tick") {

TEST_CASE("a disconnected opponent who does not reconnect in time forfeits") {
    GameRoom room("room-1");
    room.join("alice");
    room.join("bob");

    room.on_disconnect("alice");
    CHECK_FALSE(room.tick(GameRoom::kDisconnectTimeoutMs - 1).has_value());
    CHECK_FALSE(room.engine().game_over());

    std::optional<Color> loser = room.tick(1);
    REQUIRE(loser.has_value());
    CHECK(*loser == Color::w);
    CHECK(room.engine().game_over());
}

TEST_CASE("reconnecting in time cancels the countdown") {
    GameRoom room("room-1");
    room.join("alice");
    room.join("bob");

    room.on_disconnect("alice");
    room.on_reconnect("alice");

    CHECK_FALSE(room.tick(GameRoom::kDisconnectTimeoutMs + 1000).has_value());
    CHECK_FALSE(room.engine().game_over());
}

TEST_CASE("a spectator disconnecting does not start a countdown") {
    GameRoom room("room-1");
    room.join("alice");
    room.join("bob");
    room.join("carol"); // spectator

    room.on_disconnect("carol");
    CHECK_FALSE(room.tick(GameRoom::kDisconnectTimeoutMs + 1000).has_value());
}

TEST_CASE("a second disconnect resets the countdown to its full duration") {
    GameRoom room("room-1");
    room.join("alice");
    room.join("bob");

    room.on_disconnect("alice");
    room.tick(GameRoom::kDisconnectTimeoutMs - 100); // almost timed out
    room.on_disconnect("alice"); // reconnected off-screen, then disconnected again

    CHECK_FALSE(room.tick(GameRoom::kDisconnectTimeoutMs - 100).has_value()); // fresh countdown, not yet expired
}

}

TEST_SUITE("GameRoom::resign") {

TEST_CASE("resigning ends the game immediately with no countdown") {
    GameRoom room("room-1");
    room.join("alice");
    room.join("bob");

    room.resign("alice");
    CHECK(room.engine().game_over());
}

TEST_CASE("a spectator cannot resign") {
    GameRoom room("room-1");
    room.join("alice");
    room.join("bob");
    room.join("carol");

    room.resign("carol");
    CHECK_FALSE(room.engine().game_over());
}

}

TEST_SUITE("GameRoom::has_player / all_players / color_of_player") {

TEST_CASE("has_player is true for opponents and spectators alike") {
    GameRoom room("room-1");
    room.join("alice");
    room.join("bob");
    room.join("carol");

    CHECK(room.has_player("alice"));
    CHECK(room.has_player("carol"));
    CHECK_FALSE(room.has_player("nobody"));
}

TEST_CASE("all_players lists both opponents and every spectator") {
    GameRoom room("room-1");
    room.join("alice");
    room.join("bob");
    room.join("carol");

    auto players = room.all_players();
    CHECK(players.size() == 3);
}

TEST_CASE("color_of_player reports each opponent's assigned color and nullopt for a spectator") {
    GameRoom room("room-1");
    room.join("alice");
    room.join("bob");
    room.join("carol");

    REQUIRE(room.color_of_player("alice").has_value());
    CHECK(*room.color_of_player("alice") == Color::w);
    REQUIRE(room.color_of_player("bob").has_value());
    CHECK(*room.color_of_player("bob") == Color::b);
    CHECK_FALSE(room.color_of_player("carol").has_value());
}

}
