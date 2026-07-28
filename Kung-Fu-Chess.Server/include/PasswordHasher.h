#pragma once

#include <string>

// Salted password hashing (§4.2 — "never plaintext or reversible encryption").
//
// Substitution note: the plan calls for argon2id (preferred) or bcrypt
// (acceptable); this machine has no vcpkg/package manager available to pull
// either in. This instead vendors a small SHA-256 (B-Con/crypto-algorithms,
// public domain) and builds PBKDF2-HMAC-SHA256 on top — a NIST-approved,
// well-understood KDF, but its vendored source explicitly disclaims
// side-channel hardening. Swap for a real argon2id/libsodium binding via
// vcpkg before this touches production credentials.
namespace PasswordHasher {

// Returns "<hex salt>$<iterations>$<hex derived key>" — a fresh random salt every call.
std::string hash_password(const std::string& password);

// Re-derives using stored's embedded salt/iteration count and compares in constant time.
bool verify_password(const std::string& password, const std::string& stored);

} // namespace PasswordHasher
