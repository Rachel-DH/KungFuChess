#pragma once

#include <string>
#include <vector>

// Minimal WebSocket client over raw Winsock2 — the counterpart to
// WebSocketServer, substituting for a real client-side WS library (§3).
class WsClient {
public:
    WsClient();
    ~WsClient();

    WsClient(const WsClient&) = delete;
    WsClient& operator=(const WsClient&) = delete;

    // Blocking connect + opening handshake (a one-time action, not part of
    // the hot per-frame path); false on any failure.
    bool connect(const std::string& host, unsigned short port, const std::string& path = "/ws");

    void disconnect();
    bool is_connected() const { return connected_; }

    // Queues payload as one masked text frame; false if not connected.
    bool send_text(const std::string& payload);

    // Non-blocking: every complete text message received since the last poll() (often empty).
    std::vector<std::string> poll();

private:
    uintptr_t socket_handle_;
    bool connected_ = false;
    std::vector<unsigned char> recv_buffer_;
};
