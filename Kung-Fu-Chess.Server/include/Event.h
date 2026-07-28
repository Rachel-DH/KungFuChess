#pragma once

#include <string>
#include <variant>

#include "model/Position.h"
#include "utils/Types.h"

// One-off, transient room occurrences published to a room's EventBus (§4.4).
// Distinct from GAME_STATE: losing an event is tolerable (a missed sound
// cue), unlike board/cooldown/score state, which must never be lost.
struct CaptureEvent {
    Position at;
    PieceType captured_type;
    Color captured_color;
};

// Published for any game-ending result — king capture, resignation, or a
// disconnect-timeout forfeit — not literal checkmate (this variant has none).
struct CheckmateEvent {
    Color losing_color;
};

struct PlayerDisconnectedEvent {
    std::string player_id;
};

using Event = std::variant<CaptureEvent, CheckmateEvent, PlayerDisconnectedEvent>;
