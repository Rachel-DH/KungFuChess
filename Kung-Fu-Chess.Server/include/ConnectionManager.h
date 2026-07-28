#pragma once

#include <optional>
#include <string>
#include <unordered_map>

#include "WebSocketServer.h"

// Bridges raw WebSocketServer connection ids to player identities (§4.2):
// add/remove, send-to-player, track players, disconnect detection. Knows
// nothing about rooms or game rules.
class ConnectionManager {
public:
    explicit ConnectionManager(WebSocketServer& server);

    // Associates connection_id with player_id (e.g. once LOGIN succeeds). A
    // player_id can only be identified with one live connection at a time —
    // identifying it again replaces the previous connection's mapping.
    void identify(int connection_id, const std::string& player_id);

    std::optional<std::string> player_for(int connection_id) const;
    std::optional<int> connection_for(const std::string& player_id) const;

    // False if player_id has no live connection.
    bool send_to_player(const std::string& player_id, const std::string& payload);

    // Forgets connection_id's identity (call from WebSocketServer's
    // on_disconnect); returns the player_id that was on it, if any.
    std::optional<std::string> forget(int connection_id);

private:
    WebSocketServer& server_;
    std::unordered_map<int, std::string> player_by_connection_;
    std::unordered_map<std::string, int> connection_by_player_;
};
