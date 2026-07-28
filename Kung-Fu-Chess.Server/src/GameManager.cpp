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
