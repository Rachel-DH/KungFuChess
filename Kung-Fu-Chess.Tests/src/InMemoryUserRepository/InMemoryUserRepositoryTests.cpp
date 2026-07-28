#include "ThirdParty/doctest.h"

#include "InMemoryUserRepository.h"

TEST_SUITE("InMemoryUserRepository::register_user") {

TEST_CASE("a new username registers successfully") {
    InMemoryUserRepository repo;
    CHECK(repo.register_user("alice", "password123"));
}

TEST_CASE("registering an already-taken username fails") {
    InMemoryUserRepository repo;
    repo.register_user("alice", "password123");
    CHECK_FALSE(repo.register_user("alice", "a-different-password"));
}

}

TEST_SUITE("InMemoryUserRepository::authenticate") {

TEST_CASE("the correct username/password authenticates and returns the starting rating") {
    InMemoryUserRepository repo;
    repo.register_user("alice", "password123");

    auto record = repo.authenticate("alice", "password123");
    REQUIRE(record.has_value());
    CHECK(record->username == "alice");
    CHECK(record->rating == RatingManager::kStartingRating);
}

TEST_CASE("an incorrect password fails authentication") {
    InMemoryUserRepository repo;
    repo.register_user("alice", "password123");
    CHECK_FALSE(repo.authenticate("alice", "wrong-password").has_value());
}

TEST_CASE("an unknown username fails authentication") {
    InMemoryUserRepository repo;
    CHECK_FALSE(repo.authenticate("nobody", "anything").has_value());
}

}

TEST_SUITE("InMemoryUserRepository::update_rating") {

TEST_CASE("updating a user's rating is reflected on the next authenticate") {
    InMemoryUserRepository repo;
    repo.register_user("alice", "password123");
    repo.update_rating("alice", 1300);

    auto record = repo.authenticate("alice", "password123");
    REQUIRE(record.has_value());
    CHECK(record->rating == 1300);
}

}
