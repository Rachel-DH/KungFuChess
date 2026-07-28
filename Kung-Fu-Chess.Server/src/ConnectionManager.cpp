#include "ConnectionManager.h"

ConnectionManager::ConnectionManager(WebSocketServer& server) : server_(server) {
}

void ConnectionManager::identify(int connection_id, const std::string& player_id) {
    auto existing_connection = connection_by_player_.find(player_id);
    if (existing_connection != connection_by_player_.end()) {
        player_by_connection_.erase(existing_connection->second);
    }
    player_by_connection_[connection_id] = player_id;
    connection_by_player_[player_id] = connection_id;
}

std::optional<std::string> ConnectionManager::player_for(int connection_id) const {
    auto it = player_by_connection_.find(connection_id);
    return it == player_by_connection_.end() ? std::nullopt : std::optional<std::string>(it->second);
}

std::optional<int> ConnectionManager::connection_for(const std::string& player_id) const {
    auto it = connection_by_player_.find(player_id);
    return it == connection_by_player_.end() ? std::nullopt : std::optional<int>(it->second);
}

bool ConnectionManager::send_to_player(const std::string& player_id, const std::string& payload) {
    auto connection_id = connection_for(player_id);
    if (!connection_id.has_value()) {
        return false;
    }
    return server_.send_text(*connection_id, payload);
}

std::optional<std::string> ConnectionManager::forget(int connection_id) {
    auto it = player_by_connection_.find(connection_id);
    if (it == player_by_connection_.end()) {
        return std::nullopt;
    }
    std::string player_id = it->second;
    connection_by_player_.erase(player_id);
    player_by_connection_.erase(it);
    return player_id;
}
