#pragma once

#include <functional>
#include <string>

// WebSocket server, backed by websocketpp + standalone Asio (vendored in
// third_party/). Accepts connections and exchanges text frames; knows
// nothing about rooms, game rules, or the wire protocol carried over it.
//
// Non-blocking by design: poll() drives whatever I/O is ready and returns
// immediately, so it can be called once per game-loop tick without a slow or
// stalled client stalling ticking for every other room (architecture_plan.md
// §4.3's non-blocking-broadcast requirement).
class WebSocketServer {
public:
    using ConnectHandler = std::function<void(int connection_id)>;
    using MessageHandler = std::function<void(int connection_id, const std::string& message)>;
    using DisconnectHandler = std::function<void(int connection_id)>;

    // Binds and listens immediately; throws std::runtime_error on failure.
    explicit WebSocketServer(unsigned short port);
    ~WebSocketServer();

    WebSocketServer(const WebSocketServer&) = delete;
    WebSocketServer& operator=(const WebSocketServer&) = delete;

    void on_connect(ConnectHandler handler);
    void on_message(MessageHandler handler);
    void on_disconnect(DisconnectHandler handler);

    // Services whatever I/O is ready (accepting connections, completing handshakes, delivering
    // frames, flushing queued sends) and returns immediately; never blocks. Call once per tick.
    void poll();

    // Queues payload as one text frame to connection_id. False if connection_id is unknown
    // (already disconnected, or never existed).
    bool send_text(int connection_id, const std::string& payload);

    void disconnect_client(int connection_id);

    unsigned short port() const;

private:
    struct Impl;
    Impl* impl_;
};
