#include "sprite_manager.h"

#include <unordered_set>
#include <utility>

#include "piece_naming.h"
#include "sprite_frame_counter.h"

SpriteManager::SpriteManager(std::string pieces_dir, int cell_width, int cell_height)
    : pieces_dir_(std::move(pieces_dir)),
      cell_width_(cell_width),
      cell_height_(cell_height),
      anim_config_provider_(pieces_dir_)
{
}

Img& SpriteManager::piece_image(PieceType type, Color color, PiecePhase phase, int frame_index) {
    std::string code = piece_naming::piece_code(type, color);
    std::string key = code + "/" + piece_naming::phase_dir_name(phase) + "/" + std::to_string(frame_index);
    auto cached = piece_images_.find(key);
    if (cached != piece_images_.end()) return cached->second;

    Img sprite;
    std::string full_path = pieces_dir_ + "/" + code + "/states/" + piece_naming::phase_dir_name(phase) +
                        "/sprites/" + std::to_string(frame_index + 1) + ".png";

    sprite.read(full_path, { cell_width_, cell_height_ }, true);
    return piece_images_.emplace(key, std::move(sprite)).first->second;
}

Img& SpriteManager::sprite_for(int piece_id, PieceType type, Color color, PiecePhase phase) {
    auto phase_it = piece_current_phase_.find(piece_id);
    bool phase_changed = phase_it == piece_current_phase_.end() || phase_it->second != phase;

    if (phase_changed) {
        const AnimationStateConfig& config = anim_config_provider_.config_for(type, color, phase);
        std::string sprites_dir = pieces_dir_ + "/" + piece_naming::piece_code(type, color) +
                                   "/states/" + piece_naming::phase_dir_name(phase) + "/sprites";
        int frame_count = sprite_frame_counter::count_frames(sprites_dir);
        piece_animations_.insert_or_assign(piece_id, SpriteAnimation::from_config(config, frame_count));
        piece_current_phase_[piece_id] = phase;
    }

    const SpriteAnimation& anim = piece_animations_.at(piece_id);
    return piece_image(type, color, phase, anim.frame_index());
}

bool SpriteManager::advance_all(int elapsed_ms) {
    bool any_frame_changed = false;
    for (auto& entry : piece_animations_) {
        SpriteAnimation& anim = entry.second;
        int frame_before = anim.frame_index();
        anim.advance(elapsed_ms);
        if (anim.frame_index() != frame_before) any_frame_changed = true;
    }
    return any_frame_changed;
}

void SpriteManager::prune(const std::vector<int>& live_ids) {
    std::unordered_set<int> live(live_ids.begin(), live_ids.end());

    for (auto it = piece_animations_.begin(); it != piece_animations_.end();) {
        if (live.find(it->first) == live.end()) {
            it = piece_animations_.erase(it);
        } else {
            ++it;
        }
    }

    for (auto it = piece_current_phase_.begin(); it != piece_current_phase_.end();) {
        if (live.find(it->first) == live.end()) {
            it = piece_current_phase_.erase(it);
        } else {
            ++it;
        }
    }
}
