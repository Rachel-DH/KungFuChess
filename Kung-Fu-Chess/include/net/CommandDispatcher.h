#pragma once

#include "model/GameEngine.h"
#include "net/Command.h"

namespace CommandDispatcher {

// Forwards a Command to the matching GameEngine request; false if GameEngine
// rejected it (illegal move, game over, piece not selectable, etc.).
bool dispatch(const Command& command, GameEngine& engine);

} // namespace CommandDispatcher
