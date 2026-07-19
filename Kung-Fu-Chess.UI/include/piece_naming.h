#pragma once

#include <string>

#include "Types.h"

// Shared naming rules for sprite asset lookup, matching ext_src/pieces2's layout.
namespace piece_naming
{
    // Throws std::invalid_argument if `type` isn't a recognized PieceType.
    char type_letter(PieceType type);

    // "<type letter><color letter>", e.g. "BB" for a black bishop.
    std::string piece_code(PieceType type, Color color);

    // Returns the directory name for a given piece phase.
    std::string phase_dir_name(PiecePhase phase);

} // namespace piece_naming
