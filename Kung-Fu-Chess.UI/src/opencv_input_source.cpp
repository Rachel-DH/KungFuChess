#include "opencv_input_source.h"

#include <utility>

namespace {

constexpr int kEscapeKey = 27;

} // namespace

OpenCvInputSource::OpenCvInputSource(std::string window_name)
    : window_name_(std::move(window_name)) {
    // Created up front so the mouse callback is guaranteed to attach here, not left to Renderer::draw()'s later cv::imshow() on the same name.
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
    // Required every frame or the window appears frozen: OpenCV only pumps its event queue during a wait call.
    int key = cv::waitKey(1);
    if (key == kEscapeKey) {
        return true;
    }
    return cv::getWindowProperty(window_name_, cv::WND_PROP_VISIBLE) < 1;
}
