#pragma once

#include <optional>
#include <utility>

// Abstract source of pixel-coordinate clicks on the rendered board, so
// main() can poll for input without depending on any concrete (OpenCV-
// backed) implementation.
class IInputSource {
public:
    virtual ~IInputSource() = default;

    // The most recent left-click's pixel coordinates since the last poll,
    // or nullopt if there's been none. Non-blocking; buffers at most one
    // pending click.
    virtual std::optional<std::pair<int, int>> poll_click() = 0;

    // True if the user asked to quit (e.g. pressed Escape or closed the
    // window) since the last poll. Non-blocking; also responsible for
    // pumping whatever event queue the concrete input source relies on, so
    // callers must call this once per frame regardless of whether they
    // care about the result.
    virtual bool poll_quit() = 0;
};
