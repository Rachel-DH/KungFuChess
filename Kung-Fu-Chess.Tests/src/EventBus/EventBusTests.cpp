#include "ThirdParty/doctest.h"

#include <vector>

#include "EventBus.h"

TEST_SUITE("EventBus::publish") {

TEST_CASE("a single subscriber receives a published event") {
    EventBus bus;
    int received = 0;
    bus.subscribe([&](const Event&) { received++; });

    bus.publish(CaptureEvent{ Position{ 0, 0 }, PieceType::P, Color::b });
    CHECK(received == 1);
}

TEST_CASE("every subscriber receives every published event, in subscription order") {
    EventBus bus;
    std::vector<int> order;
    bus.subscribe([&](const Event&) { order.push_back(1); });
    bus.subscribe([&](const Event&) { order.push_back(2); });

    bus.publish(CheckmateEvent{ Color::w });
    CHECK(order == std::vector<int>{ 1, 2 });
}

TEST_CASE("a subscriber can inspect which event variant it received") {
    EventBus bus;
    bool saw_disconnect = false;
    bus.subscribe([&](const Event& event) {
        saw_disconnect = std::holds_alternative<PlayerDisconnectedEvent>(event);
    });

    bus.publish(PlayerDisconnectedEvent{ "player-1" });
    CHECK(saw_disconnect);
}

TEST_CASE("publishing with no subscribers does nothing") {
    EventBus bus;
    CHECK_NOTHROW(bus.publish(CheckmateEvent{ Color::b }));
}

}
