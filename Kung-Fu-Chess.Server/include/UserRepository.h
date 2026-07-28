#pragma once

#include <optional>
#include <string>

#include "RatingManager.h"

struct UserRecord {
    std::string username;
    std::string password_hash; // PasswordHasher output; never plaintext or reversible (§4.2)
    int rating = RatingManager::kStartingRating;
};

// Abstract so the Command Layer and tests depend on UserRepository, never on
// SQLite directly (§4.2) — swappable and independently mockable.
class UserRepository {
public:
    virtual ~UserRepository() = default;

    // False if username is already taken.
    virtual bool register_user(const std::string& username, const std::string& password) = 0;

    // nullopt if the username doesn't exist or the password doesn't match.
    virtual std::optional<UserRecord> authenticate(const std::string& username, const std::string& password) = 0;

    virtual void update_rating(const std::string& username, int new_rating) = 0;
};
