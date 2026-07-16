#include <chrono>

#include "Controller.h"
#include "InputSourceFactory.h"
#include "RenderLoop.h"
#include "RendererFactory.h"

int main() {
    auto last_tick = std::chrono::steady_clock::now();

    Controller controller = Controller::standard_start();
    auto renderer = make_renderer();
    auto input = make_input_source();
    RenderLoop loop(controller, *renderer, *input);

    bool running = true;
    while (running) {
        auto now = std::chrono::steady_clock::now();
        int elapsed_ms = static_cast<int>(
            std::chrono::duration_cast<std::chrono::milliseconds>(now - last_tick).count());
        last_tick = now;

        running = loop.tick(elapsed_ms);
    }
}
