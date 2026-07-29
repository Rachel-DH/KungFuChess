#pragma once

#include <string>
#include <vector>

// WebSocket client, backed by websocketpp + standalone Asio (vendored in
// third_party/). Connects to a WebSocketServer and exchanges text frames;
// knows nothing about rooms, game rules, or the wire protocol carried over it.
//
// Non-blocking by design, matching WebSocketServer: poll() drives whatever
// I/O is ready and returns immediately. Call once per client-loop tick.
class WsClient {
public:
    WsClient();
    ~WsClient();

    WsClient(const WsClient&) = delete;
    WsClient& operator=(const WsClient&) = delete;

    // Starts connecting to ws://host:port/path; the connection completes asynchronously (see
    // is_connected()). Throws std::runtime_error if the connection attempt can't even be started
    // (e.g. malformed host).
    void connect(const std::string& host, unsigned short port, const std::string& path = "/");

    void disconnect();

    // True once the WebSocket handshake has completed; false before connect() or after disconnect()
    // (including a disconnect the server initiated).
    bool is_connected() const;

    // Queues payload as one text frame. False if not currently connected.
    bool send_text(const std::string& payload);

    // Services whatever I/O is ready and returns immediately; never blocks. Call once per tick.
    void poll();

    // Every text message received since the last call, in arrival order; drains the queue.
    std::vector<std::string> drain_messages();

private:
    struct Impl;
    Impl* impl_;
};
