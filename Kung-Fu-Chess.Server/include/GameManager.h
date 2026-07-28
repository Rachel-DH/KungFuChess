#pragma once

#include <memory>
#include <string>
#include <unordered_map>

#include "GameRoom.h"

// Registry of rooms only (§4.2) — create/get/remove. No gameplay knowledge;
// callers reach GameEngine only through the GameRoom they look up here.
class GameManager {
public:
    // Creates the room if room_id is new; otherwise returns the existing one (§7: the first joiner becomes the room's creator).
    GameRoom& get_or_create(const std::string& room_id);

    // nullptr if no room with that id exists.
    GameRoom* find(const std::string& room_id);

    void remove(const std::string& room_id);

    std::size_t room_count() const { return rooms_.size(); }

private:
    std::unordered_map<std::string, std::unique_ptr<GameRoom>> rooms_;
};
