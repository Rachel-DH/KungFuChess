#include "WebSocketServer.h"

#include <stdexcept>

#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h>

#include "Base64.h"
#include "Sha1.h"
#include "WebSocketFrame.h"

#pragma comment(lib, "ws2_32.lib")

namespace {

constexpr const char* kWebSocketGuid = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";

std::string compute_accept_key(const std::string& client_key) {
    auto digest = Sha1::hash(client_key + kWebSocketGuid);
    return Base64::encode(digest.data(), digest.size());
}

// Bare-bones HTTP header extraction: finds "Header-Name: value\r\n" case-insensitively.
std::string find_header(const std::string& request, const std::string& header_name) {
    size_t pos = request.find(header_name);
    if (pos == std::string::npos) {
        return "";
    }
    size_t value_start = request.find(':', pos) + 1;
    while (value_start < request.size() && request[value_start] == ' ') {
        ++value_start;
    }
    size_t value_end = request.find("\r\n", value_start);
    return request.substr(value_start, value_end - value_start);
}

void set_non_blocking(SOCKET s) {
    u_long mode = 1;
    ioctlsocket(s, FIONBIO, &mode);
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

WebSocketServer::WebSocketServer(unsigned short port) {
    SOCKET listen_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listen_socket == INVALID_SOCKET) {
        throw std::runtime_error("WebSocketServer: socket() failed");
    }

    int reuse = 1;
    setsockopt(listen_socket, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char*>(&reuse), sizeof(reuse));

    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    if (bind(listen_socket, reinterpret_cast<sockaddr*>(&address), sizeof(address)) == SOCKET_ERROR
        || listen(listen_socket, SOMAXCONN) == SOCKET_ERROR) {
        closesocket(listen_socket);
        throw std::runtime_error("WebSocketServer: bind/listen failed");
    }

    set_non_blocking(listen_socket);
    listen_socket_handle_ = static_cast<uintptr_t>(listen_socket);
}

WebSocketServer::~WebSocketServer() {
    for (auto& [id, connection] : connections_) {
        closesocket(static_cast<SOCKET>(connection.socket_handle));
    }
    closesocket(static_cast<SOCKET>(listen_socket_handle_));
}

void WebSocketServer::on_connect(ConnectHandler handler) {
    on_connect_ = std::move(handler);
}

void WebSocketServer::on_message(MessageHandler handler) {
    on_message_ = std::move(handler);
}

void WebSocketServer::on_disconnect(DisconnectHandler handler) {
    on_disconnect_ = std::move(handler);
}

void WebSocketServer::accept_new_connections() {
    for (;;) {
        SOCKET client_socket = accept(static_cast<SOCKET>(listen_socket_handle_), nullptr, nullptr);
        if (client_socket == INVALID_SOCKET) {
            break; // WSAEWOULDBLOCK: no pending connection right now
        }
        set_non_blocking(client_socket);
        int id = next_connection_id_++;
        connections_.emplace(id, Connection{ static_cast<uintptr_t>(client_socket) });
    }
}

bool WebSocketServer::try_complete_handshake(Connection& connection) {
    size_t header_end = connection.handshake_buffer.find("\r\n\r\n");
    if (header_end == std::string::npos) {
        return false; // request not fully arrived yet
    }

    std::string key = find_header(connection.handshake_buffer, "Sec-WebSocket-Key");
    std::string accept_key = compute_accept_key(key);

    std::string response = "HTTP/1.1 101 Switching Protocols\r\n"
        "Upgrade: websocket\r\n"
        "Connection: Upgrade\r\n"
        "Sec-WebSocket-Accept: " + accept_key + "\r\n\r\n";

    send(static_cast<SOCKET>(connection.socket_handle), response.data(), static_cast<int>(response.size()), 0);
    return true;
}

void WebSocketServer::flush_send_buffer(Connection& connection) {
    while (!connection.send_buffer.empty()) {
        int sent = send(static_cast<SOCKET>(connection.socket_handle),
            reinterpret_cast<const char*>(connection.send_buffer.data()),
            static_cast<int>(connection.send_buffer.size()), 0);
        if (sent <= 0) {
            break; // WSAEWOULDBLOCK (or a real error, caught next poll's recv) — try again next poll()
        }
        connection.send_buffer.erase(connection.send_buffer.begin(), connection.send_buffer.begin() + sent);
    }
}

void WebSocketServer::service_connection(int id, Connection& connection) {
    flush_send_buffer(connection);

    char recv_chunk[4096];
    int received = recv(static_cast<SOCKET>(connection.socket_handle), recv_chunk, sizeof(recv_chunk), 0);
    if (received == 0) {
        close_connection(id);
        return;
    }
    if (received < 0) {
        int error = WSAGetLastError();
        if (error != WSAEWOULDBLOCK) {
            close_connection(id);
        }
        return;
    }

    if (!connection.handshake_complete) {
        connection.handshake_buffer.append(recv_chunk, received);
        if (try_complete_handshake(connection)) {
            connection.handshake_complete = true;
            connection.handshake_buffer.clear();
            if (on_connect_) {
                on_connect_(id);
            }
        }
        return;
    }

    connection.recv_buffer.insert(connection.recv_buffer.end(), recv_chunk, recv_chunk + received);
    for (;;) {
        auto frame = WebSocketFrame::try_decode_frame(connection.recv_buffer);
        if (!frame.has_value()) {
            break;
        }
        if (frame->opcode == WebSocketFrame::kOpcodeClose) {
            close_connection(id);
            return;
        }
        if (frame->opcode == WebSocketFrame::kOpcodeText && on_message_) {
            on_message_(id, frame->payload);
        }
    }
}

void WebSocketServer::close_connection(int id) {
    auto it = connections_.find(id);
    if (it == connections_.end()) {
        return;
    }
    closesocket(static_cast<SOCKET>(it->second.socket_handle));
    connections_.erase(it);
    if (on_disconnect_) {
        on_disconnect_(id);
    }
}

void WebSocketServer::poll() {
    accept_new_connections();

    // Copy ids first: service_connection() may erase from connections_ (close_connection).
    std::vector<int> ids;
    ids.reserve(connections_.size());
    for (const auto& [id, connection] : connections_) {
        ids.push_back(id);
    }
    for (int id : ids) {
        auto it = connections_.find(id);
        if (it != connections_.end()) {
            service_connection(id, it->second);
        }
    }
}

bool WebSocketServer::send_text(int connection_id, const std::string& payload) {
    auto it = connections_.find(connection_id);
    if (it == connections_.end() || !it->second.handshake_complete) {
        return false;
    }
    auto frame = WebSocketFrame::encode_server_text_frame(payload);
    it->second.send_buffer.insert(it->second.send_buffer.end(), frame.begin(), frame.end());
    flush_send_buffer(it->second);
    return true;
}

void WebSocketServer::disconnect_client(int connection_id) {
    close_connection(connection_id);
}
