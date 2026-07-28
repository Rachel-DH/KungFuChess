#include "ThirdParty/doctest.h"

#include "PasswordHasher.h"

TEST_SUITE("PasswordHasher::hash_password / verify_password") {

TEST_CASE("the correct password verifies against its own hash") {
    std::string stored = PasswordHasher::hash_password("correct horse battery staple");
    CHECK(PasswordHasher::verify_password("correct horse battery staple", stored));
}

TEST_CASE("an incorrect password does not verify") {
    std::string stored = PasswordHasher::hash_password("correct horse battery staple");
    CHECK_FALSE(PasswordHasher::verify_password("wrong password", stored));
}

TEST_CASE("hashing the same password twice produces different stored strings") {
    std::string first = PasswordHasher::hash_password("same-password");
    std::string second = PasswordHasher::hash_password("same-password");
    CHECK(first != second); // random salt each call
}

TEST_CASE("the stored hash never contains the plaintext password") {
    std::string stored = PasswordHasher::hash_password("hunter2");
    CHECK(stored.find("hunter2") == std::string::npos);
}

}
