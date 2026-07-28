#include "ThirdParty/doctest.h"

#include <algorithm>

#include "GameManager.h"

TEST_SUITE("GameManager::get_or_create") {

TEST_CASE("a new room id creates a new room") {
    GameManager manager;
    GameRoom& room = manager.get_or_create("room-1");
    CHECK(room.id() == "room-1");
    CHECK(manager.room_count() == 1);
}

TEST_CASE("requesting the same room id twice returns the same room instance") {
    GameManager manager;
    GameRoom& first = manager.get_or_create("room-1");
    first.join("alice");

    GameRoom& second = manager.get_or_create("room-1");
    CHECK(&first == &second);
    CHECK(manager.room_count() == 1);
    CHECK(second.join("alice").role == PlayerRole::Opponent); // alice already holds the White slot
}

}

TEST_SUITE("GameManager::find") {

TEST_CASE("find returns nullptr for a room id that was never created") {
    GameManager manager;
    CHECK(manager.find("no-such-room") == nullptr);
}

TEST_CASE("find returns the existing room once it has been created") {
    GameManager manager;
    manager.get_or_create("room-1");
    CHECK(manager.find("room-1") != nullptr);
}

}

TEST_SUITE("GameManager::remove") {

TEST_CASE("removing a room makes it findable no longer") {
    GameManager manager;
    manager.get_or_create("room-1");
    manager.remove("room-1");
    CHECK(manager.find("room-1") == nullptr);
    CHECK(manager.room_count() == 0);
}

}

TEST_SUITE("GameManager::room_ids") {

TEST_CASE("lists every currently-registered room id") {
    GameManager manager;
    manager.get_or_create("room-1");
    manager.get_or_create("room-2");

    auto ids = manager.room_ids();
    CHECK(ids.size() == 2);
    CHECK(std::find(ids.begin(), ids.end(), "room-1") != ids.end());
    CHECK(std::find(ids.begin(), ids.end(), "room-2") != ids.end());
}

}

TEST_SUITE("GameManager::find_room_for_player") {

TEST_CASE("finds the room a player holds an opponent slot in") {
    GameManager manager;
    manager.get_or_create("room-1").join("alice");

    GameRoom* room = manager.find_room_for_player("alice");
    REQUIRE(room != nullptr);
    CHECK(room->id() == "room-1");
}

TEST_CASE("finds the room a player is spectating") {
    GameManager manager;
    GameRoom& room = manager.get_or_create("room-1");
    room.join("alice");
    room.join("bob");
    room.join("carol"); // spectator

    GameRoom* found = manager.find_room_for_player("carol");
    REQUIRE(found != nullptr);
    CHECK(found->id() == "room-1");
}

TEST_CASE("returns nullptr for a player in no room") {
    GameManager manager;
    manager.get_or_create("room-1").join("alice");
    CHECK(manager.find_room_for_player("nobody") == nullptr);
}

}
