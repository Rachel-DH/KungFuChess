#include "net/CommandDispatcher.h"

namespace CommandDispatcher {

bool dispatch(const Command& command, GameEngine& engine) {
    return std::visit([&engine](const auto& c) {
        using T = std::decay_t<decltype(c)>;
        if constexpr (std::is_same_v<T, MoveCommand>) {
            return engine.request_move(c.start, c.dest);
        } else {
            return engine.request_jump(c.cell);
        }
    }, command);
}

} // namespace CommandDispatcher
