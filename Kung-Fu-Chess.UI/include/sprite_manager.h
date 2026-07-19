#pragma once

#include <string>
#include <unordered_map>

#include "Types.h"
#include "img.h"
#include "piece_animation_config_provider.h"
#include "sprite_animation.h"

// Owns per-piece animation state (phase/frame tracking) and sprite image loading/caching;
// extracted out of Renderer so animation bookkeeping doesn't depend on how/where pieces
// get drawn.
class SpriteManager {
public:
    SpriteManager(std::string pieces_dir, int cell_width, int cell_height);

    // Returns the sprite frame to show for `piece_id` right now, advancing its animation
    // by `elapsed_ms`; starts a fresh animation whenever `phase` differs from the piece's
    // last known phase.
    Img& sprite_for(int piece_id, PieceType type, Color color, PiecePhase phase, int elapsed_ms);

private:
    // Returns the sprite for (type, color, phase, frame_index), loading and caching it on
    // first use so repeated calls don't re-read the same file.
    Img& piece_image(PieceType type, Color color, PiecePhase phase, int frame_index);

    std::string pieces_dir_;
    int cell_width_;
    int cell_height_;

    std::unordered_map<std::string, Img> piece_images_;

    PieceAnimationConfigProvider anim_config_provider_;
    std::unordered_map<int /*piece_id*/, SpriteAnimation> piece_animations_;
    std::unordered_map<int /*piece_id*/, PiecePhase> piece_current_phase_;
};
