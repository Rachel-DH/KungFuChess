#include "piece_animation_config_provider.h"

#include <stdexcept>
#include <utility>

#include "piece_naming.h"

namespace {

std::string phase_dir_name(PiecePhase phase) {
    switch (phase) {
        case PiecePhase::Idle: return "idle";
        case PiecePhase::Move: return "move";
        case PiecePhase::Jump: return "jump";
    }
    throw std::invalid_argument("Unknown PiecePhase");
}

}  // namespace

PieceAnimationConfigProvider::PieceAnimationConfigProvider(std::string pieces_dir)
    : pieces_dir_(std::move(pieces_dir)) {
}

const AnimationStateConfig& PieceAnimationConfigProvider::config_for(PieceType type, Color color,
                                                                       PiecePhase phase) {
    std::string code = piece_naming::piece_code(type, color);
    std::string state = phase_dir_name(phase);
    std::string key = code + "/" + state;

    auto cached = cache_.find(key);
    if (cached != cache_.end()) {
        return cached->second;
    }

    std::string path = pieces_dir_ + "/" + code + "/states/" + state + "/config.json";
    return cache_.emplace(key, read_animation_state_config(path)).first->second;
}
