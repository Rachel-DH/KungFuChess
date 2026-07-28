#pragma once

#include <array>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

// Minimal RFC 6455 framing — substitutes for uWebSockets (§4.1), which
// needs a package manager this machine doesn't have. Deliberately scoped
// down: single-frame text messages only (opcode 0x1), no fragmentation
// (continuation frames), and control frames (ping/pong/close, 0x9/0xA/0x8)
// are decoded but left for the caller to act on — every JSON command and
// GAME_STATE broadcast in this protocol fits in one frame.
namespace WebSocketFrame {

constexpr unsigned char kOpcodeText = 0x1;
constexpr unsigned char kOpcodeClose = 0x8;
constexpr unsigned char kOpcodePing = 0x9;
constexpr unsigned char kOpcodePong = 0xA;

struct DecodedFrame {
    unsigned char opcode;
    std::string payload;
};

// Server -> client frames are sent unmasked (RFC 6455 §5.1: masking is
// mandatory only in the client -> server direction).
std::vector<unsigned char> encode_server_text_frame(const std::string& payload);

// Client -> server frames must be masked; mask_key is exposed as a
// parameter purely so tests can produce deterministic output — real
// callers should pass 4 cryptographically-unpredictable bytes.
std::vector<unsigned char> encode_client_text_frame(const std::string& payload,
    const std::array<unsigned char, 4>& mask_key);

// Attempts to decode exactly one frame from the front of `buffer`. On
// success, erases the consumed bytes and returns the frame; returns
// nullopt (leaving buffer untouched) if it doesn't yet hold a complete frame.
std::optional<DecodedFrame> try_decode_frame(std::vector<unsigned char>& buffer);

} // namespace WebSocketFrame
