#pragma once

#include <functional>
#include <vector>

#include "Event.h"

// Room-scoped pub/sub (§4.4) — every GameRoom owns its own instance; this is
// deliberately not a global singleton (see architecture_plan.md §4.4 for why).
class EventBus {
public:
    using Subscriber = std::function<void(const Event&)>;

    void subscribe(Subscriber subscriber);

    // Fans out to every subscriber, in subscription order.
    void publish(const Event& event) const;

private:
    std::vector<Subscriber> subscribers_;
};
