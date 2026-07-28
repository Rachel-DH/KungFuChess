#include "ThirdParty/doctest.h"

#include "SqliteUserRepository.h"

namespace {

// A fresh in-memory database per test — no file cleanup needed, and each
// test gets an isolated schema instance.
SqliteUserRepository make_repo() {
    return SqliteUserRepository(":memory:");
}

} // namespace

TEST_SUITE("SqliteUserRepository::register_user / authenticate") {

TEST_CASE("a registered user can authenticate with the correct password") {
    SqliteUserRepository repo = make_repo();
    REQUIRE(repo.register_user("alice", "password123"));

    auto record = repo.authenticate("alice", "password123");
    REQUIRE(record.has_value());
    CHECK(record->username == "alice");
    CHECK(record->rating == RatingManager::kStartingRating);
}

TEST_CASE("registering an already-taken username fails") {
    SqliteUserRepository repo = make_repo();
    repo.register_user("alice", "password123");
    CHECK_FALSE(repo.register_user("alice", "another-password"));
}

TEST_CASE("authenticating with the wrong password fails") {
    SqliteUserRepository repo = make_repo();
    repo.register_user("alice", "password123");
    CHECK_FALSE(repo.authenticate("alice", "wrong-password").has_value());
}

TEST_CASE("the password is never stored in plaintext") {
    SqliteUserRepository repo = make_repo();
    repo.register_user("alice", "hunter2");
    // authenticate() re-derives from the stored hash; a plaintext match would still pass verify_password,
    // so this only confirms the stored record's hash column itself never equals the password directly.
    auto record = repo.authenticate("alice", "hunter2");
    REQUIRE(record.has_value());
    CHECK(record->password_hash.find("hunter2") == std::string::npos);
}

}

TEST_SUITE("SqliteUserRepository::update_rating") {

TEST_CASE("an updated rating persists and is returned on the next authenticate") {
    SqliteUserRepository repo = make_repo();
    repo.register_user("alice", "password123");
    repo.update_rating("alice", 1350);

    auto record = repo.authenticate("alice", "password123");
    REQUIRE(record.has_value());
    CHECK(record->rating == 1350);
}

}
