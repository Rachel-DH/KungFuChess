#pragma once

#include <optional>
#include <string>
#include <utility>

#include <opencv2/opencv.hpp>

#include "IInputSource.h"

// Polls left mouse-button clicks on an OpenCV window via cv::setMouseCallback; poll_click() drains the last buffered click, so a click is reported to exactly one caller.
class OpenCvInputSource : public IInputSource {
public:
    // `window_name` must match the name Renderer::draw() passes to cv::imshow(), or clicks on that window won't reach this callback.
    explicit OpenCvInputSource(std::string window_name);

    std::optional<std::pair<int, int>> poll_click() override;

    // Pumps OpenCV's event queue via cv::waitKey(1) and reports whether Escape was pressed or the window was closed since the last poll.
    bool poll_quit() override;

private:
    // cv::setMouseCallback needs a C-style function pointer; `userdata` carries the OpenCvInputSource instance through to handle_mouse().
    static void on_mouse(int event, int x, int y, int flags, void* userdata);
    void handle_mouse(int event, int x, int y);

    std::string window_name_;
    std::optional<std::pair<int, int>> pending_click_;
};
