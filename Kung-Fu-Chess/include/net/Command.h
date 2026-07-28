#pragma once

#include <variant>

#include "model/Position.h"

// The typed shape every transport (local, text-protocol, or networked)
// converges on before reaching GameEngine. See CommandDispatcher.
struct MoveCommand {
    Position start;
    Position dest;
};

struct JumpCommand {
    Position cell;
};

using Command = std::variant<MoveCommand, JumpCommand>;
