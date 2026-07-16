#include "opencv_input_source.h"

#include <utility>

namespace {

constexpr int kEscapeKey = 27;

} // namespace

OpenCvInputSource::OpenCvInputSource(std::string window_name)
    : window_name_(std::move(window_name)) {
    // cv::setMouseCallback only attaches if the window already exists;
    // create it up front so the callback is guaranteed to register here
    // rather than depending on Renderer::draw() having run first. Renderer's
    // later cv::imshow() calls on the same name reuse this window.
    cv::namedWindow(window_name_);
    cv::setMouseCallback(window_name_, &OpenCvInputSource::on_mouse, this);
}

void OpenCvInputSource::on_mouse(int event, int x, int y, int /*flags*/, void* userdata) {
    static_cast<OpenCvInputSource*>(userdata)->handle_mouse(event, x, y);
}

void OpenCvInputSource::handle_mouse(int event, int x, int y) {
    if (event == cv::EVENT_LBUTTONDOWN) {
        pending_click_ = std::make_pair(x, y);
    }
}

std::optional<std::pair<int, int>> OpenCvInputSource::poll_click() {
    std::optional<std::pair<int, int>> click = pending_click_;
    pending_click_.reset();
    return click;
}

bool OpenCvInputSource::poll_quit() {
    // Required every frame or the window appears frozen: OpenCV only
    // pumps its window/mouse event queue while a wait function is running.
    int key = cv::waitKey(1);
    if (key == kEscapeKey) {
        return true;
    }
    return cv::getWindowProperty(window_name_, cv::WND_PROP_VISIBLE) < 1;
}
