#pragma once

#include "animation_state_config.h"

// Turns (frame_count, frames_per_sec, is_loop) plus accumulated elapsed time into "which frame index to show" and "has a one-shot animation finished" — a generic sprite-sheet cycling primitive.
class SpriteAnimation {
public:
    SpriteAnimation(int frame_count, int frames_per_sec, bool is_loop);

    // The real asset layout always ships 5 sprite frames per state.
    static constexpr int kStandardFrameCount = 5;
    static SpriteAnimation from_config(const AnimationStateConfig& config, int frame_count = kStandardFrameCount);

    // Advances the animation by elapsed_ms, consuming whole frames and carrying over leftover time (no drift across many small calls); a no-op once finished().
    void advance(int elapsed_ms);

    int frame_index() const;

    // True once a non-looping animation has played through its last frame; always false for a looping animation.
    bool finished() const;

    // Returns to frame 0 and clears finished(), e.g. when the piece's phase changes.
    void reset();

private:
    int frame_count_;
    int frames_per_sec_;
    bool is_loop_;
    int frame_index_ = 0;
    long long elapsed_in_frame_ms_ = 0;
    bool finished_ = false;
};
