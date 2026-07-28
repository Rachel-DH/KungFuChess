#include "ThirdParty/doctest.h"

#include <atomic>
#include <chrono>
#include <thread>

#include "WebSocketServer.h"
#include "WsClient.h"

namespace {

// WsClient::connect() blocks on the handshake response, so it runs on its
// own thread while the test thread keeps calling server.poll() — mirroring
// how a real client blocks on connect() while the server's single-threaded
// loop keeps ticking (§4.3).
bool connect_client_pumping_server(WsClient& client, WebSocketServer& server, unsigned short port) {
    std::atomic<bool> connect_finished{ false };
    bool connect_result = false;
    std::thread connector([&] {
        connect_result = client.connect("127.0.0.1", port);
        connect_finished = true;
    });

    auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(3);
    while (!connect_finished.load() && std::chrono::steady_clock::now() < deadline) {
        server.poll();
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
    connector.join();
    return connect_result;
}

template <typename Predicate>
bool pump_until(WebSocketServer& server, Predicate predicate, std::chrono::milliseconds timeout) {
    auto deadline = std::chrono::steady_clock::now() + timeout;
    while (std::chrono::steady_clock::now() < deadline) {
        server.poll();
        if (predicate()) {
            return true;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
    return predicate();
}

template <typename Predicate>
bool pump_client_until(WsClient& client, Predicate predicate, std::chrono::milliseconds timeout) {
    auto deadline = std::chrono::steady_clock::now() + timeout;
    while (std::chrono::steady_clock::now() < deadline) {
        if (predicate()) {
            return true;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
    return predicate();
}

} // namespace

TEST_SUITE("WebSocketServer / WsClient — loopback") {

TEST_CASE("a client completes the opening handshake and the server's on_connect fires") {
    WebSocketServer server(17845);
    bool connected_fired = false;
    server.on_connect([&](int) { connected_fired = true; });

    WsClient client;
    bool connected = connect_client_pumping_server(client, server, 17845);

    REQUIRE(connected);
    CHECK(client.is_connected());
    CHECK(connected_fired);
}

TEST_CASE("a text message sent by the client reaches the server's on_message handler") {
    WebSocketServer server(17846);
    std::string received_payload;
    server.on_message([&](int, const std::string& payload) { received_payload = payload; });

    WsClient client;
    REQUIRE(connect_client_pumping_server(client, server, 17846));

    client.send_text("hello server");
    bool got_message = pump_until(
        server, [&] { return !received_payload.empty(); }, std::chrono::seconds(2));

    REQUIRE(got_message);
    CHECK(received_payload == "hello server");
}

TEST_CASE("a text message sent by the server reaches the client's poll()") {
    WebSocketServer server(17847);
    int connection_id = -1;
    server.on_connect([&](int id) { connection_id = id; });

    WsClient client;
    REQUIRE(connect_client_pumping_server(client, server, 17847));
    REQUIRE(pump_until(
        server, [&] { return connection_id != -1; }, std::chrono::seconds(1)));

    REQUIRE(server.send_text(connection_id, "hello client"));

    std::vector<std::string> received;
    bool got_message = pump_client_until(
        client,
        [&] {
            auto messages = client.poll();
            received.insert(received.end(), messages.begin(), messages.end());
            return !received.empty();
        },
        std::chrono::seconds(2));

    REQUIRE(got_message);
    CHECK(received.front() == "hello client");
}

TEST_CASE("disconnecting a client fires the server's on_disconnect handler") {
    WebSocketServer server(17848);
    bool disconnect_fired = false;
    server.on_disconnect([&](int) { disconnect_fired = true; });

    WsClient client;
    REQUIRE(connect_client_pumping_server(client, server, 17848));

    client.disconnect();
    bool saw_disconnect = pump_until(server, [&] { return disconnect_fired; }, std::chrono::seconds(2));
    CHECK(saw_disconnect);
}

}
