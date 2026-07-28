#include "GameManager.h"

GameRoom& GameManager::get_or_create(const std::string& room_id) {
    auto it = rooms_.find(room_id);
    if (it == rooms_.end()) {
        it = rooms_.emplace(room_id, std::make_unique<GameRoom>(room_id)).first;
    }
    return *it->second;
}

GameRoom* GameManager::find(const std::string& room_id) {
    auto it = rooms_.find(room_id);
    return it == rooms_.end() ? nullptr : it->second.get();
}

void GameManager::remove(const std::string& room_id) {
    rooms_.erase(room_id);
}

std::vector<std::string> GameManager::room_ids() const {
    std::vector<std::string> ids;
    ids.reserve(rooms_.size());
    for (const auto& [id, room] : rooms_) {
        ids.push_back(id);
    }
    return ids;
}

GameRoom* GameManager::find_room_for_player(const std::string& player_id) {
    for (auto& [id, room] : rooms_) {
        if (room->has_player(player_id)) {
            return room.get();
        }
    }
    return nullptr;
}
