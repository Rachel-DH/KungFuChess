#include "WebSocketFrame.h"

namespace WebSocketFrame {

namespace {

void append_length(std::vector<unsigned char>& out, uint64_t len, bool masked) {
    unsigned char mask_bit = masked ? 0x80 : 0x00;
    if (len <= 125) {
        out.push_back(static_cast<unsigned char>(mask_bit | len));
    } else if (len <= 0xFFFF) {
        out.push_back(static_cast<unsigned char>(mask_bit | 126));
        out.push_back(static_cast<unsigned char>((len >> 8) & 0xFF));
        out.push_back(static_cast<unsigned char>(len & 0xFF));
    } else {
        out.push_back(static_cast<unsigned char>(mask_bit | 127));
        for (int shift = 56; shift >= 0; shift -= 8) {
            out.push_back(static_cast<unsigned char>((len >> shift) & 0xFF));
        }
    }
}

} // namespace

std::vector<unsigned char> encode_server_text_frame(const std::string& payload) {
    std::vector<unsigned char> out;
    out.push_back(0x80 | kOpcodeText); // FIN=1, opcode=text
    append_length(out, payload.size(), /*masked=*/false);
    out.insert(out.end(), payload.begin(), payload.end());
    return out;
}

std::vector<unsigned char> encode_client_text_frame(const std::string& payload,
    const std::array<unsigned char, 4>& mask_key) {
    std::vector<unsigned char> out;
    out.push_back(0x80 | kOpcodeText);
    append_length(out, payload.size(), /*masked=*/true);
    out.insert(out.end(), mask_key.begin(), mask_key.end());
    for (size_t i = 0; i < payload.size(); ++i) {
        out.push_back(static_cast<unsigned char>(payload[i]) ^ mask_key[i % 4]);
    }
    return out;
}

std::optional<DecodedFrame> try_decode_frame(std::vector<unsigned char>& buffer) {
    if (buffer.size() < 2) {
        return std::nullopt;
    }

    unsigned char byte0 = buffer[0];
    unsigned char byte1 = buffer[1];
    bool masked = (byte1 & 0x80) != 0;
    uint64_t payload_len = byte1 & 0x7F;
    size_t offset = 2;

    if (payload_len == 126) {
        if (buffer.size() < offset + 2) {
            return std::nullopt;
        }
        payload_len = (static_cast<uint64_t>(buffer[offset]) << 8) | buffer[offset + 1];
        offset += 2;
    } else if (payload_len == 127) {
        if (buffer.size() < offset + 8) {
            return std::nullopt;
        }
        payload_len = 0;
        for (int i = 0; i < 8; ++i) {
            payload_len = (payload_len << 8) | buffer[offset + i];
        }
        offset += 8;
    }

    std::array<unsigned char, 4> mask_key{ 0, 0, 0, 0 };
    if (masked) {
        if (buffer.size() < offset + 4) {
            return std::nullopt;
        }
        for (int i = 0; i < 4; ++i) {
            mask_key[i] = buffer[offset + i];
        }
        offset += 4;
    }

    if (buffer.size() < offset + payload_len) {
        return std::nullopt; // frame not fully arrived yet
    }

    std::string payload(static_cast<size_t>(payload_len), '\0');
    for (uint64_t i = 0; i < payload_len; ++i) {
        unsigned char b = buffer[offset + i];
        if (masked) {
            b = static_cast<unsigned char>(b ^ mask_key[i % 4]);
        }
        payload[static_cast<size_t>(i)] = static_cast<char>(b);
    }

    size_t total_frame_size = offset + static_cast<size_t>(payload_len);
    buffer.erase(buffer.begin(), buffer.begin() + static_cast<long>(total_frame_size));

    return DecodedFrame{ static_cast<unsigned char>(byte0 & 0x0F), payload };
}

} // namespace WebSocketFrame
