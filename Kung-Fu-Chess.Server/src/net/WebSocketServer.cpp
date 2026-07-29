#include "net/WebSocketServer.h"

#include <map>
#include <unordered_map>

#pragma warning(push, 0)
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>
#pragma warning(pop)

struct WebSocketServer::Impl {
    using Server = websocketpp::server<websocketpp::config::asio>;

    explicit Impl(unsigned short p) : port(p) {
        server.clear_access_channels(websocketpp::log::alevel::all);
        server.clear_error_channels(websocketpp::log::elevel::all);
        server.init_asio();
        server.set_reuse_addr(true);

        server.set_open_handler([this](websocketpp::connection_hdl hdl) { handle_open(hdl); });
        server.set_close_handler([this](websocketpp::connection_hdl hdl) { handle_close(hdl); });
        server.set_message_handler(
            [this](websocketpp::connection_hdl hdl, Server::message_ptr msg) { handle_message(hdl, msg); });

        server.listen(port);
        server.start_accept();
    }

    void handle_open(websocketpp::connection_hdl hdl) {
        int id = next_id++;
        id_to_hdl[id] = hdl;
        hdl_to_id[hdl] = id;
        if (on_connect) {
            on_connect(id);
        }
    }

    void handle_close(websocketpp::connection_hdl hdl) {
        auto it = hdl_to_id.find(hdl);
        if (it == hdl_to_id.end()) {
            return;
        }
        int id = it->second;
        hdl_to_id.erase(it);
        id_to_hdl.erase(id);
        if (on_disconnect) {
            on_disconnect(id);
        }
    }

    void handle_message(websocketpp::connection_hdl hdl, const Server::message_ptr& msg) {
        auto it = hdl_to_id.find(hdl);
        if (it == hdl_to_id.end()) {
            return;
        }
        if (on_message) {
            on_message(it->second, msg->get_payload());
        }
    }

    Server server;
    unsigned short port;
    int next_id = 1;
    std::unordered_map<int, websocketpp::connection_hdl> id_to_hdl;
    std::map<websocketpp::connection_hdl, int, std::owner_less<websocketpp::connection_hdl>> hdl_to_id;

    WebSocketServer::ConnectHandler on_connect;
    WebSocketServer::MessageHandler on_message;
    WebSocketServer::DisconnectHandler on_disconnect;
};

WebSocketServer::WebSocketServer(unsigned short port) : impl_(new Impl(port)) {
}

WebSocketServer::~WebSocketServer() {
    delete impl_;
}

void WebSocketServer::on_connect(ConnectHandler handler) {
    impl_->on_connect = std::move(handler);
}

void WebSocketServer::on_message(MessageHandler handler) {
    impl_->on_message = std::move(handler);
}

void WebSocketServer::on_disconnect(DisconnectHandler handler) {
    impl_->on_disconnect = std::move(handler);
}

void WebSocketServer::poll() {
    impl_->server.get_io_service().poll();
}

bool WebSocketServer::send_text(int connection_id, const std::string& payload) {
    auto it = impl_->id_to_hdl.find(connection_id);
    if (it == impl_->id_to_hdl.end()) {
        return false;
    }
    websocketpp::lib::error_code ec;
    impl_->server.send(it->second, payload, websocketpp::frame::opcode::text, ec);
    return !ec;
}

void WebSocketServer::disconnect_client(int connection_id) {
    auto it = impl_->id_to_hdl.find(connection_id);
    if (it == impl_->id_to_hdl.end()) {
        return;
    }
    websocketpp::lib::error_code ec;
    impl_->server.close(it->second, websocketpp::close::status::normal, "", ec);
}

unsigned short WebSocketServer::port() const {
    return impl_->port;
}
