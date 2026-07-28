#pragma once

#include <array>
#include <string>

// Thin C++ wrapper around the vendored sha1.h — kept in its own
// translation unit so sha1.h's WORD/BYTE typedefs never collide with
// windows.h's (WebSocketServer.cpp/WsClient.cpp need both SHA-1 and
// winsock2.h; sha1.h itself is never safe to include alongside windows.h).
namespace Sha1 {

std::array<unsigned char, 20> hash(const std::string& data);

} // namespace Sha1
