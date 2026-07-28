#pragma once

#include <string>
#include <variant>

#include "net/Command.h"

// Every inbound message the wire protocol accepts, beyond the in-game
// MoveCommand/JumpCommand already handled by the Command Layer (§4.1):
// session actions the server main loop handles directly, before a command
// ever reaches a room's GameEngine.
struct RegisterMessage {
    std::string username;
    std::string password;
};

struct LoginMessage {
    std::string username;
    std::string password;
};

struct JoinRoomMessage {
    std::string room_name;
};

struct QuickMatchMessage {
};

struct ResignMessage {
};

using ClientMessage = std::variant<RegisterMessage, LoginMessage, JoinRoomMessage, QuickMatchMessage, ResignMessage,
    MoveCommand, JumpCommand>;
