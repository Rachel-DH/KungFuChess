#pragma once

#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

// Minimal WebSocket server over raw Winsock2 — substitutes for uWebSockets
// (§4.1; no package manager on this machine). Matches §4.3's single
// game-loop-thread model: poll() never blocks, so a slow/stalled client
// can never stall ticking for every other room (§4.3's non-blocking-
// broadcast requirement). One connection = one accepted TCP socket that
// has completed the RFC 6455 opening handshake.
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

    // Accepts new connections, advances in-progress handshakes, decodes any
    // fully-arrived frames, and flushes queued outbound bytes — all
    // non-blocking. Call once per game-loop tick.
    void poll();

    // Queues payload as one text frame; sends what it can immediately and
    // buffers the rest for the next poll() rather than blocking. False if
    // connection_id is unknown (already disconnected).
    bool send_text(int connection_id, const std::string& payload);

    void disconnect_client(int connection_id);

private:
    struct Connection {
        uintptr_t socket_handle;
        bool handshake_complete = false;
        std::string handshake_buffer;
        std::vector<unsigned char> recv_buffer;
        std::vector<unsigned char> send_buffer;
    };

    uintptr_t listen_socket_handle_;
    std::unordered_map<int, Connection> connections_;
    int next_connection_id_ = 1;
    ConnectHandler on_connect_;
    MessageHandler on_message_;
    DisconnectHandler on_disconnect_;

    void accept_new_connections();
    void service_connection(int id, Connection& connection);
    bool try_complete_handshake(Connection& connection);
    void flush_send_buffer(Connection& connection);
    void close_connection(int id);
};
