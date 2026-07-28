#include "PasswordHasher.h"

#include <array>
#include <cstring>
#include <iomanip>
#include <random>
#include <sstream>
#include <vector>

extern "C" {
#include "ThirdParty/sha256.h"
}

namespace PasswordHasher {

namespace {

constexpr int kSaltBytes = 16;
constexpr int kKeyBytes = SHA256_BLOCK_SIZE; // 32
constexpr int kIterations = 100000;

std::string to_hex(const unsigned char* data, size_t len) {
    std::ostringstream oss;
    for (size_t i = 0; i < len; ++i) {
        oss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(data[i]);
    }
    return oss.str();
}

std::vector<unsigned char> from_hex(const std::string& hex) {
    std::vector<unsigned char> out(hex.size() / 2);
    for (size_t i = 0; i < out.size(); ++i) {
        out[i] = static_cast<unsigned char>(std::stoi(hex.substr(i * 2, 2), nullptr, 16));
    }
    return out;
}

void hmac_sha256(const unsigned char* key, size_t key_len, const unsigned char* data, size_t data_len,
    unsigned char out[SHA256_BLOCK_SIZE]) {
    unsigned char key_block[64] = { 0 };
    if (key_len > 64) {
        SHA256_CTX ctx;
        sha256_init(&ctx);
        sha256_update(&ctx, key, static_cast<size_t>(key_len));
        sha256_final(&ctx, key_block);
    } else {
        std::memcpy(key_block, key, key_len);
    }

    unsigned char o_key_pad[64];
    unsigned char i_key_pad[64];
    for (int i = 0; i < 64; ++i) {
        o_key_pad[i] = static_cast<unsigned char>(key_block[i] ^ 0x5c);
        i_key_pad[i] = static_cast<unsigned char>(key_block[i] ^ 0x36);
    }

    unsigned char inner_hash[SHA256_BLOCK_SIZE];
    SHA256_CTX ctx;
    sha256_init(&ctx);
    sha256_update(&ctx, i_key_pad, 64);
    sha256_update(&ctx, data, data_len);
    sha256_final(&ctx, inner_hash);

    sha256_init(&ctx);
    sha256_update(&ctx, o_key_pad, 64);
    sha256_update(&ctx, inner_hash, SHA256_BLOCK_SIZE);
    sha256_final(&ctx, out);
}

// One 32-byte PBKDF2 block is exactly kKeyBytes, so only block index 1 is ever needed.
std::array<unsigned char, kKeyBytes> pbkdf2(const std::string& password, const std::vector<unsigned char>& salt,
    int iterations) {
    std::vector<unsigned char> salt_and_index(salt);
    salt_and_index.insert(salt_and_index.end(), { 0, 0, 0, 1 }); // INT(1), big-endian

    unsigned char u[SHA256_BLOCK_SIZE];
    hmac_sha256(reinterpret_cast<const unsigned char*>(password.data()), password.size(), salt_and_index.data(),
        salt_and_index.size(), u);

    std::array<unsigned char, kKeyBytes> result{};
    std::memcpy(result.data(), u, kKeyBytes);

    for (int i = 1; i < iterations; ++i) {
        unsigned char next[SHA256_BLOCK_SIZE];
        hmac_sha256(reinterpret_cast<const unsigned char*>(password.data()), password.size(), u, SHA256_BLOCK_SIZE,
            next);
        for (int b = 0; b < kKeyBytes; ++b) {
            result[b] ^= next[b];
        }
        std::memcpy(u, next, SHA256_BLOCK_SIZE);
    }
    return result;
}

std::vector<unsigned char> random_salt(int num_bytes) {
    std::random_device rd;
    std::vector<unsigned char> salt(static_cast<size_t>(num_bytes));
    for (auto& b : salt) {
        b = static_cast<unsigned char>(rd() & 0xFF);
    }
    return salt;
}

} // namespace

std::string hash_password(const std::string& password) {
    std::vector<unsigned char> salt = random_salt(kSaltBytes);
    auto derived = pbkdf2(password, salt, kIterations);
    return to_hex(salt.data(), salt.size()) + "$" + std::to_string(kIterations) + "$"
        + to_hex(derived.data(), derived.size());
}

bool verify_password(const std::string& password, const std::string& stored) {
    size_t first_dollar = stored.find('$');
    size_t second_dollar = first_dollar == std::string::npos ? std::string::npos : stored.find('$', first_dollar + 1);
    if (first_dollar == std::string::npos || second_dollar == std::string::npos) {
        return false;
    }

    std::string salt_hex = stored.substr(0, first_dollar);
    int iterations = std::stoi(stored.substr(first_dollar + 1, second_dollar - first_dollar - 1));
    std::string key_hex = stored.substr(second_dollar + 1);

    std::vector<unsigned char> salt = from_hex(salt_hex);
    auto derived = pbkdf2(password, salt, iterations);
    std::string derived_hex = to_hex(derived.data(), derived.size());

    if (derived_hex.size() != key_hex.size()) {
        return false;
    }
    unsigned char diff = 0;
    for (size_t i = 0; i < derived_hex.size(); ++i) {
        diff |= static_cast<unsigned char>(derived_hex[i]) ^ static_cast<unsigned char>(key_hex[i]);
    }
    return diff == 0;
}

} // namespace PasswordHasher
