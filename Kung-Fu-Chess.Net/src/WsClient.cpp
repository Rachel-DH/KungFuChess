#include "WsClient.h"

#include <array>
#include <random>

#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h>

#include "Base64.h"
#include "WebSocketFrame.h"

#pragma comment(lib, "ws2_32.lib")

namespace {

void set_non_blocking(SOCKET s) {
    u_long mode = 1;
    ioctlsocket(s, FIONBIO, &mode);
}

std::string random_base64_key() {
    std::random_device rd;
    unsigned char bytes[16];
    for (auto& b : bytes) {
        b = static_cast<unsigned char>(rd() & 0xFF);
    }
    return Base64::encode(bytes, sizeof(bytes));
}

struct WsaGuard {
    WsaGuard() {
        WSADATA data;
        WSAStartup(MAKEWORD(2, 2), &data);
    }
    ~WsaGuard() { WSACleanup(); }
};

WsaGuard g_wsa_guard; // one process-wide WSAStartup/WSACleanup pair

} // namespace

WsClient::WsClient() : socket_handle_(static_cast<uintptr_t>(INVALID_SOCKET)) {
}

WsClient::~WsClient() {
    disconnect();
}

bool WsClient::connect(const std::string& host, unsigned short port, const std::string& path) {
    SOCKET s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (s == INVALID_SOCKET) {
        return false;
    }

    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_port = htons(port);
    if (InetPtonA(AF_INET, host.c_str(), &address.sin_addr) != 1) {
        addrinfo hints{};
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;
        addrinfo* resolved = nullptr;
        if (getaddrinfo(host.c_str(), nullptr, &hints, &resolved) != 0 || resolved == nullptr) {
            closesocket(s);
            return false;
        }
        address.sin_addr = reinterpret_cast<sockaddr_in*>(resolved->ai_addr)->sin_addr;
        freeaddrinfo(resolved);
    }

    if (::connect(s, reinterpret_cast<sockaddr*>(&address), sizeof(address)) == SOCKET_ERROR) {
        closesocket(s);
        return false;
    }

    DWORD recv_timeout_ms = 3000; // bounds the handshake wait; poll() runs non-blocking afterward
    setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, reinterpret_cast<const char*>(&recv_timeout_ms), sizeof(recv_timeout_ms));

    std::string request = "GET " + path + " HTTP/1.1\r\n"
        "Host: " + host + "\r\n"
        "Upgrade: websocket\r\n"
        "Connection: Upgrade\r\n"
        "Sec-WebSocket-Key: " + random_base64_key() + "\r\n"
        "Sec-WebSocket-Version: 13\r\n\r\n";
    send(s, request.data(), static_cast<int>(request.size()), 0);

    std::string response;
    char chunk[1024];
    for (;;) {
        int received = recv(s, chunk, sizeof(chunk), 0);
        if (received <= 0) {
            closesocket(s);
            return false;
        }
        response.append(chunk, received);
        if (response.find("\r\n\r\n") != std::string::npos) {
            break;
        }
        if (response.size() > 8192) { // malformed/oversized handshake response
            closesocket(s);
            return false;
        }
    }

    if (response.find("101") == std::string::npos) {
        closesocket(s);
        return false;
    }

    set_non_blocking(s);
    socket_handle_ = static_cast<uintptr_t>(s);
    connected_ = true;
    recv_buffer_.clear();
    return true;
}

void WsClient::disconnect() {
    if (connected_) {
        closesocket(static_cast<SOCKET>(socket_handle_));
        connected_ = false;
    }
}

bool WsClient::send_text(const std::string& payload) {
    if (!connected_) {
        return false;
    }

    std::random_device rd;
    std::array<unsigned char, 4> mask_key{};
    for (auto& b : mask_key) {
        b = static_cast<unsigned char>(rd() & 0xFF);
    }
    auto frame = WebSocketFrame::encode_client_text_frame(payload, mask_key);

    size_t total_sent = 0;
    while (total_sent < frame.size()) {
        int sent = send(static_cast<SOCKET>(socket_handle_),
            reinterpret_cast<const char*>(frame.data() + total_sent), static_cast<int>(frame.size() - total_sent), 0);
        if (sent <= 0) {
            break; // best-effort — a stalled outbound send isn't retried further here
        }
        total_sent += static_cast<size_t>(sent);
    }
    return true;
}

std::vector<std::string> WsClient::poll() {
    std::vector<std::string> messages;
    if (!connected_) {
        return messages;
    }

    char chunk[4096];
    for (;;) {
        int received = recv(static_cast<SOCKET>(socket_handle_), chunk, sizeof(chunk), 0);
        if (received > 0) {
            recv_buffer_.insert(recv_buffer_.end(), chunk, chunk + received);
            continue;
        }
        if (received == 0) {
            connected_ = false;
        } else if (WSAGetLastError() != WSAEWOULDBLOCK) {
            connected_ = false;
        }
        break;
    }

    for (;;) {
        auto frame = WebSocketFrame::try_decode_frame(recv_buffer_);
        if (!frame.has_value()) {
            break;
        }
        if (frame->opcode == WebSocketFrame::kOpcodeText) {
            messages.push_back(frame->payload);
        } else if (frame->opcode == WebSocketFrame::kOpcodeClose) {
            connected_ = false;
        }
    }
    return messages;
}
