#include "Base64.h"

namespace Base64 {

namespace {
constexpr char kAlphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
}

std::string encode(const unsigned char* data, size_t len) {
    std::string out;
    out.reserve((len + 2) / 3 * 4);

    size_t i = 0;
    while (i + 3 <= len) {
        unsigned int chunk = (data[i] << 16) | (data[i + 1] << 8) | data[i + 2];
        out += kAlphabet[(chunk >> 18) & 0x3F];
        out += kAlphabet[(chunk >> 12) & 0x3F];
        out += kAlphabet[(chunk >> 6) & 0x3F];
        out += kAlphabet[chunk & 0x3F];
        i += 3;
    }

    size_t remaining = len - i;
    if (remaining == 1) {
        unsigned int chunk = data[i] << 16;
        out += kAlphabet[(chunk >> 18) & 0x3F];
        out += kAlphabet[(chunk >> 12) & 0x3F];
        out += "==";
    } else if (remaining == 2) {
        unsigned int chunk = (data[i] << 16) | (data[i + 1] << 8);
        out += kAlphabet[(chunk >> 18) & 0x3F];
        out += kAlphabet[(chunk >> 12) & 0x3F];
        out += kAlphabet[(chunk >> 6) & 0x3F];
        out += "=";
    }

    return out;
}

} // namespace Base64
