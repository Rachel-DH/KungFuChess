#include "ThirdParty/doctest.h"

#include "Matchmaker.h"

TEST_SUITE("Matchmaker::find_quick_match") {

TEST_CASE("the first player to queue gets no match yet") {
    GameManager manager;
    Matchmaker matchmaker(manager);

    CHECK_FALSE(matchmaker.find_quick_match("alice", 1200).has_value());
}

TEST_CASE("a second player within ELO range is paired into a new room") {
    GameManager manager;
    Matchmaker matchmaker(manager);

    matchmaker.find_quick_match("alice", 1200);
    std::optional<std::string> room_id = matchmaker.find_quick_match("bob", 1250);

    REQUIRE(room_id.has_value());
    GameRoom* room = manager.find(*room_id);
    REQUIRE(room != nullptr);
    CHECK(room->join("alice").role == PlayerRole::Opponent); // alice already holds White
    CHECK(room->join("bob").role == PlayerRole::Opponent);   // bob already holds Black
}

TEST_CASE("a player far outside the ELO range is not matched and queues instead") {
    GameManager manager;
    Matchmaker matchmaker(manager, /*elo_range=*/100);

    matchmaker.find_quick_match("alice", 1200);
    CHECK_FALSE(matchmaker.find_quick_match("bob", 1500).has_value());
}

TEST_CASE("a cancelled quick match is no longer available to be paired with") {
    GameManager manager;
    Matchmaker matchmaker(manager);

    matchmaker.find_quick_match("alice", 1200);
    matchmaker.cancel_quick_match("alice");

    CHECK_FALSE(matchmaker.find_quick_match("bob", 1200).has_value());
}

}
