#include "ThirdParty/doctest.h"

#include "ConnectionManager.h"

TEST_SUITE("ConnectionManager::identify / player_for / connection_for") {

TEST_CASE("an identified connection can be looked up by connection id or player id") {
    WebSocketServer server(17849);
    ConnectionManager manager(server);

    manager.identify(1, "alice");
    CHECK(manager.player_for(1) == "alice");
    CHECK(manager.connection_for("alice") == 1);
}

TEST_CASE("an unidentified connection id returns nullopt") {
    WebSocketServer server(17850);
    ConnectionManager manager(server);
    CHECK_FALSE(manager.player_for(99).has_value());
}

TEST_CASE("re-identifying the same player id under a new connection replaces the old mapping") {
    WebSocketServer server(17851);
    ConnectionManager manager(server);

    manager.identify(1, "alice");
    manager.identify(2, "alice"); // alice reconnected on a new socket

    CHECK_FALSE(manager.player_for(1).has_value());
    CHECK(manager.connection_for("alice") == 2);
}

}

TEST_SUITE("ConnectionManager::forget") {

TEST_CASE("forgetting a connection returns its player id and clears both mappings") {
    WebSocketServer server(17852);
    ConnectionManager manager(server);
    manager.identify(1, "alice");

    auto forgotten = manager.forget(1);
    REQUIRE(forgotten.has_value());
    CHECK(*forgotten == "alice");
    CHECK_FALSE(manager.player_for(1).has_value());
    CHECK_FALSE(manager.connection_for("alice").has_value());
}

TEST_CASE("forgetting a connection that was never identified returns nullopt") {
    WebSocketServer server(17853);
    ConnectionManager manager(server);
    CHECK_FALSE(manager.forget(42).has_value());
}

}

TEST_SUITE("ConnectionManager::send_to_player") {

TEST_CASE("sending to a player with no live connection fails") {
    WebSocketServer server(17854);
    ConnectionManager manager(server);
    CHECK_FALSE(manager.send_to_player("nobody", "hello"));
}

}
