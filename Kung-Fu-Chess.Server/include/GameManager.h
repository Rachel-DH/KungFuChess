#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

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

    // Every room id currently registered — the server main loop's tick
    // iterates this every frame (§4.3: "for each room: room.tick(dt)").
    std::vector<std::string> room_ids() const;

    // nullptr if player_id isn't an opponent or spectator in any room.
    GameRoom* find_room_for_player(const std::string& player_id);

private:
    std::unordered_map<std::string, std::unique_ptr<GameRoom>> rooms_;
};
