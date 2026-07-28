#include "InMemoryUserRepository.h"

#include "PasswordHasher.h"

bool InMemoryUserRepository::register_user(const std::string& username, const std::string& password) {
    if (users_.count(username) > 0) {
        return false;
    }
    users_[username] = UserRecord{ username, PasswordHasher::hash_password(password), RatingManager::kStartingRating };
    return true;
}

std::optional<UserRecord> InMemoryUserRepository::authenticate(const std::string& username,
    const std::string& password) {
    auto it = users_.find(username);
    if (it == users_.end() || !PasswordHasher::verify_password(password, it->second.password_hash)) {
        return std::nullopt;
    }
    return it->second;
}

void InMemoryUserRepository::update_rating(const std::string& username, int new_rating) {
    auto it = users_.find(username);
    if (it != users_.end()) {
        it->second.rating = new_rating;
    }
}
