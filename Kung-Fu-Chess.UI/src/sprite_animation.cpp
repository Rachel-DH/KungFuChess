#include "sprite_animation.h"

SpriteAnimation::SpriteAnimation(int frame_count, int frames_per_sec, bool is_loop)
    : frame_count_(frame_count), frames_per_sec_(frames_per_sec), is_loop_(is_loop) {
}

SpriteAnimation SpriteAnimation::from_config(const AnimationStateConfig& config, int frame_count) {
    return SpriteAnimation(frame_count, config.frames_per_sec, config.is_loop);
}

void SpriteAnimation::advance(int elapsed_ms) {
    if (finished_) {
        return;
    }

    elapsed_in_frame_ms_ += elapsed_ms;
    int ms_per_frame = 1000 / frames_per_sec_;
    while (elapsed_in_frame_ms_ >= ms_per_frame) {
        elapsed_in_frame_ms_ -= ms_per_frame;
        ++frame_index_;
        if (frame_index_ >= frame_count_) {
            if (is_loop_) {
                frame_index_ %= frame_count_;
            } else {
                frame_index_ = frame_count_ - 1;
                finished_ = true;
                break;
            }
        }
    }
}

int SpriteAnimation::frame_index() const {
    return frame_index_;
}

bool SpriteAnimation::finished() const {
    return finished_;
}

void SpriteAnimation::reset() {
    frame_index_ = 0;
    elapsed_in_frame_ms_ = 0;
    finished_ = false;
}
