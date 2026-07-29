#include "ThirdParty/doctest.h"

#include <chrono>
#include <optional>
#include <string>
#include <thread>
#include <vector>

#include "net/WebSocketServer.h"
#include "net/WsClient.h"

namespace {

// Real network I/O is asynchronous even on localhost, so tests drive both ends' poll() in a
// loop until `condition` is satisfied or `timeout_ms` elapses, rather than assuming one poll()
// call is enough. A short sleep between iterations avoids busy-spinning the CPU while waiting
// for the OS/Asio to actually deliver bytes.
template <typename Condition>
bool poll_until(WebSocketServer& server, WsClient& client, Condition condition, int timeout_ms = 2000) {
    auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(timeout_ms);
    while (std::chrono::steady_clock::now() < deadline) {
        server.poll();
        client.poll();
        if (condition()) {
            return true;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(2));
    }
    return false;
}

// Ephemeral-ish port per test case so tests never collide even if a prior run's socket lingers
// briefly in TIME_WAIT; each TEST_CASE picks its own distinct value from this range.
constexpr unsigned short kBasePort = 17000;

} // namespace

TEST_SUITE("WebSocketServer + WsClient round trip") {

TEST_CASE("a client connecting to the server triggers on_connect with a connection id") {
    WebSocketServer server(kBasePort + 1);
    std::optional<int> connected_id;
    server.on_connect([&](int id) { connected_id = id; });

    WsClient client;
    client.connect("127.0.0.1", kBasePort + 1);

    bool got_connect = poll_until(server, client, [&] { return connected_id.has_value(); });
    REQUIRE(got_connect);
    CHECK(client.is_connected());
}

TEST_CASE("text sent from the client is delivered to the server's on_message with the exact payload") {
    WebSocketServer server(kBasePort + 2);
    std::optional<std::string> received;
    server.on_message([&](int, const std::string& message) { received = message; });

    WsClient client;
    client.connect("127.0.0.1", kBasePort + 2);
    REQUIRE(poll_until(server, client, [&] { return client.is_connected(); }));

    REQUIRE(client.send_text("hello from client"));
    REQUIRE(poll_until(server, client, [&] { return received.has_value(); }));
    CHECK(*received == "hello from client");
}

TEST_CASE("text sent from the server is delivered to the client via drain_messages") {
    WebSocketServer server(kBasePort + 3);
    std::optional<int> connected_id;
    server.on_connect([&](int id) { connected_id = id; });

    WsClient client;
    client.connect("127.0.0.1", kBasePort + 3);
    REQUIRE(poll_until(server, client, [&] { return connected_id.has_value(); }));

    REQUIRE(server.send_text(*connected_id, "hello from server"));

    std::vector<std::string> messages;
    poll_until(server, client, [&] {
        messages = client.drain_messages();
        return !messages.empty();
    });
    REQUIRE(messages.size() == 1);
    CHECK(messages[0] == "hello from server");
}

TEST_CASE("the client disconnecting triggers the server's on_disconnect for that connection id") {
    WebSocketServer server(kBasePort + 4);
    std::optional<int> connected_id;
    std::optional<int> disconnected_id;
    server.on_connect([&](int id) { connected_id = id; });
    server.on_disconnect([&](int id) { disconnected_id = id; });

    WsClient client;
    client.connect("127.0.0.1", kBasePort + 4);
    REQUIRE(poll_until(server, client, [&] { return connected_id.has_value(); }));

    client.disconnect();
    REQUIRE(poll_until(server, client, [&] { return disconnected_id.has_value(); }));
    CHECK(*disconnected_id == *connected_id);
}

TEST_CASE("send_text to an unknown connection id returns false") {
    WebSocketServer server(kBasePort + 5);
    CHECK_FALSE(server.send_text(999999, "nobody is listening"));
}

TEST_CASE("send_text on a client that never connected returns false") {
    WsClient client;
    CHECK_FALSE(client.send_text("not connected yet"));
}

}
