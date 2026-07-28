#pragma once

#include <cstddef>
#include <string>

// Encoding only — the one direction the WebSocket handshake needs
// (base64(SHA-1(key + GUID)) for Sec-WebSocket-Accept).
namespace Base64 {

std::string encode(const unsigned char* data, size_t len);

} // namespace Base64
