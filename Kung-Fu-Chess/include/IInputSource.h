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
};
