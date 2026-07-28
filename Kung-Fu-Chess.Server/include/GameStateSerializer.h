#pragma once

#include <string>
#include <vector>

#include "Event.h"
#include "model/GameEngine.h"

// Serializes the authoritative GAME_STATE broadcast (§4.1, §4.4, §6): every
// piece's board position/type/color/phase, the game-over flag, and every
// event published to the room's EventBus since the previous tick (cleared
// by the caller after each broadcast, per §4.4).
namespace GameStateSerializer {

std::string to_json(const GameEngine& engine, const std::vector<Event>& events);

} // namespace GameStateSerializer
