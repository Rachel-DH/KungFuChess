#include "SqliteUserRepository.h"

#include <stdexcept>

#include "PasswordHasher.h"
#include "ThirdParty/sqlite/sqlite3.h"

namespace {

void exec_or_throw(sqlite3* db, const char* sql) {
    char* error_message = nullptr;
    if (sqlite3_exec(db, sql, nullptr, nullptr, &error_message) != SQLITE_OK) {
        std::string message = error_message ? error_message : "unknown SQLite error";
        sqlite3_free(error_message);
        throw std::runtime_error(message);
    }
}

} // namespace

SqliteUserRepository::SqliteUserRepository(const std::string& db_path) {
    if (sqlite3_open(db_path.c_str(), &db_) != SQLITE_OK) {
        std::string message = sqlite3_errmsg(db_);
        sqlite3_close(db_);
        throw std::runtime_error("failed to open database: " + message);
    }
    exec_or_throw(db_,
        "CREATE TABLE IF NOT EXISTS users ("
        "username TEXT PRIMARY KEY,"
        "password_hash TEXT NOT NULL,"
        "rating INTEGER NOT NULL"
        ");");
}

SqliteUserRepository::~SqliteUserRepository() {
    sqlite3_close(db_);
}

bool SqliteUserRepository::register_user(const std::string& username, const std::string& password) {
    sqlite3_stmt* stmt = nullptr;
    sqlite3_prepare_v2(db_, "INSERT INTO users (username, password_hash, rating) VALUES (?, ?, ?);", -1, &stmt,
        nullptr);
    std::string hash = PasswordHasher::hash_password(password);
    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, hash.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 3, RatingManager::kStartingRating);
    int result = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return result == SQLITE_DONE; // SQLITE_CONSTRAINT on a duplicate username -> false
}

std::optional<UserRecord> SqliteUserRepository::authenticate(const std::string& username,
    const std::string& password) {
    sqlite3_stmt* stmt = nullptr;
    sqlite3_prepare_v2(db_, "SELECT password_hash, rating FROM users WHERE username = ?;", -1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT);

    std::optional<UserRecord> result;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        std::string stored_hash = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        int rating = sqlite3_column_int(stmt, 1);
        if (PasswordHasher::verify_password(password, stored_hash)) {
            result = UserRecord{ username, stored_hash, rating };
        }
    }
    sqlite3_finalize(stmt);
    return result;
}

void SqliteUserRepository::update_rating(const std::string& username, int new_rating) {
    sqlite3_stmt* stmt = nullptr;
    sqlite3_prepare_v2(db_, "UPDATE users SET rating = ? WHERE username = ?;", -1, &stmt, nullptr);
    sqlite3_bind_int(stmt, 1, new_rating);
    sqlite3_bind_text(stmt, 2, username.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
}
