#include "net/WsClient.h"

#include <stdexcept>

#pragma warning(push, 0)
#include <websocketpp/client.hpp>
#include <websocketpp/config/asio_no_tls_client.hpp>
#pragma warning(pop)

struct WsClient::Impl {
    using Client = websocketpp::client<websocketpp::config::asio_client>;

    Impl() {
        client.clear_access_channels(websocketpp::log::alevel::all);
        client.clear_error_channels(websocketpp::log::elevel::all);
        client.init_asio();

        client.set_open_handler([this](websocketpp::connection_hdl) { connected = true; });
        client.set_close_handler([this](websocketpp::connection_hdl) { connected = false; });
        client.set_fail_handler([this](websocketpp::connection_hdl) { connected = false; });
        client.set_message_handler([this](websocketpp::connection_hdl, const Client::message_ptr& msg) {
            messages.push_back(msg->get_payload());
        });
    }

    Client client;
    websocketpp::connection_hdl hdl;
    bool has_connection = false;
    bool connected = false;
    std::vector<std::string> messages;
};

WsClient::WsClient() : impl_(new Impl()) {
}

WsClient::~WsClient() {
    delete impl_;
}

void WsClient::connect(const std::string& host, unsigned short port, const std::string& path) {
    std::string uri = "ws://" + host + ":" + std::to_string(port) + path;
    websocketpp::lib::error_code ec;
    auto con = impl_->client.get_connection(uri, ec);
    if (ec) {
        throw std::runtime_error("WsClient::connect failed: " + ec.message());
    }
    impl_->hdl = con->get_handle();
    impl_->has_connection = true;
    impl_->client.connect(con);
}

void WsClient::disconnect() {
    if (!impl_->has_connection) {
        return;
    }
    websocketpp::lib::error_code ec;
    impl_->client.close(impl_->hdl, websocketpp::close::status::normal, "", ec);
}

bool WsClient::is_connected() const {
    return impl_->connected;
}

bool WsClient::send_text(const std::string& payload) {
    if (!impl_->connected) {
        return false;
    }
    websocketpp::lib::error_code ec;
    impl_->client.send(impl_->hdl, payload, websocketpp::frame::opcode::text, ec);
    return !ec;
}

void WsClient::poll() {
    impl_->client.get_io_service().poll();
}

std::vector<std::string> WsClient::drain_messages() {
    std::vector<std::string> result;
    result.swap(impl_->messages);
    return result;
}
