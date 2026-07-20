#pragma once

#include <string>
#include <unordered_map>
#include <vector>

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

    // Returns the sprite frame to show for `piece_id` right now, at its animation's
    // current frame; starts a fresh animation whenever `phase` differs from the piece's
    // last known phase. Does not advance any animation — call advance_all() for that.
    Img& sprite_for(int piece_id, PieceType type, Color color, PiecePhase phase);

    // Advances every tracked piece's animation by elapsed_ms. Returns true if any
    // animation's current frame changed as a result, so callers can tell whether a
    // redraw would actually look different.
    bool advance_all(int elapsed_ms);

    // Drops tracking state for any piece_id not in `live_ids`. piece_id is derived from
    // board position, so a moved piece leaves a stale entry under its old id behind in
    // both maps unless the caller prunes it after every draw.
    void prune(const std::vector<int>& live_ids);

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
