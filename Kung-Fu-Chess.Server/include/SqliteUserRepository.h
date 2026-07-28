#pragma once

#include <string>

#include "UserRepository.h"

struct sqlite3; // avoids leaking sqlite3.h into every UserRepository consumer

// SQLite-backed UserRepository (§4.2, §4.5). Creates its users table on
// first open if it doesn't already exist.
class SqliteUserRepository : public UserRepository {
public:
    explicit SqliteUserRepository(const std::string& db_path);
    ~SqliteUserRepository() override;

    SqliteUserRepository(const SqliteUserRepository&) = delete;
    SqliteUserRepository& operator=(const SqliteUserRepository&) = delete;

    bool register_user(const std::string& username, const std::string& password) override;
    std::optional<UserRecord> authenticate(const std::string& username, const std::string& password) override;
    void update_rating(const std::string& username, int new_rating) override;

private:
    sqlite3* db_ = nullptr;
};
