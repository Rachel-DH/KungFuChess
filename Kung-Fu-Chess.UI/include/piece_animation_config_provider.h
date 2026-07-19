#pragma once

#include <string>
#include <unordered_map>

#include "GameEngine.h"
#include "Types.h"
#include "animation_state_config.h"

// Reads and hands out each (type, color, phase)'s animation config from
// <pieces_dir>/<code>/states/<idle|move|jump>/config.json, caching each
// one after its first read — the config is fixed asset data, never
// invalidated. Holds no per-piece or per-frame state; which phase a piece
// is actually in right now is not this class's concern.
class PieceAnimationConfigProvider {
public:
    explicit PieceAnimationConfigProvider(std::string pieces_dir);

    const AnimationStateConfig& config_for(PieceType type, Color color, PiecePhase phase);

private:
    std::string pieces_dir_;
    std::unordered_map<std::string, AnimationStateConfig> cache_;
};
