#include "EventBus.h"

void EventBus::subscribe(Subscriber subscriber) {
    subscribers_.push_back(std::move(subscriber));
}

void EventBus::publish(const Event& event) const {
    for (const auto& subscriber : subscribers_) {
        subscriber(event);
    }
}
