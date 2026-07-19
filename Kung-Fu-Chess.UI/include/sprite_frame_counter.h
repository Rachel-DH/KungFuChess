#pragma once

#include <string>

// Counts animation frame files in a sprite sheet's folder.
namespace sprite_frame_counter {

// Number of entries in `sprites_dir`, each treated as one animation frame image.
// Throws std::runtime_error if the directory doesn't exist, or exists but is empty.
int count_frames(const std::string& sprites_dir);

}  // namespace sprite_frame_counter
