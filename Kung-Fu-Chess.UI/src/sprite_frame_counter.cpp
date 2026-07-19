#include "sprite_frame_counter.h"

#include <filesystem>
#include <stdexcept>

namespace sprite_frame_counter {

int count_frames(const std::string& sprites_dir) {
    if (!std::filesystem::is_directory(sprites_dir)) {
        throw std::runtime_error("Sprite directory does not exist: " + sprites_dir);
    }

    int frame_count = 0;
    for (const auto& entry : std::filesystem::directory_iterator(sprites_dir)) {
        (void)entry;
        ++frame_count;
    }

    if (frame_count == 0) {
        throw std::runtime_error("Sprite directory is empty: " + sprites_dir);
    }
    return frame_count;
}

}  // namespace sprite_frame_counter
