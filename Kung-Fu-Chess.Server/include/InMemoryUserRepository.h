#pragma once

#include <unordered_map>

#include "UserRepository.h"

// Non-persistent UserRepository — useful for tests and for running the
// server without a database file.
class InMemoryUserRepository : public UserRepository {
public:
    bool register_user(const std::string& username, const std::string& password) override;
    std::optional<UserRecord> authenticate(const std::string& username, const std::string& password) override;
    void update_rating(const std::string& username, int new_rating) override;

private:
    std::unordered_map<std::string, UserRecord> users_;
};
