#pragma once

#include <optional>
#include <string>
#include <vector>

#include "GameManager.h"

// Quick match by ELO range (§5, §7) — the "Play" path only. "Room" is a
// completely separate path (§7): joining a room by name goes straight
// through GameManager/GameRoom, never through Matchmaker.
class Matchmaker {
public:
    explicit Matchmaker(GameManager& game_manager, int elo_range = 100);

    // Enqueues player_id for a quick match. If a waiting opponent within
    // elo_range is found, both are paired into a new room (first-waiting
    // player becomes White, per §7) and that room's id is returned.
    // Returns nullopt if no suitable opponent is available yet — the caller
    // shows the "no suitable opponent found" fallback message (§5).
    std::optional<std::string> find_quick_match(const std::string& player_id, int rating);

    // Removes player_id from the waiting queue (e.g. they cancelled before being matched).
    void cancel_quick_match(const std::string& player_id);

private:
    struct WaitingPlayer {
        std::string player_id;
        int rating;
    };

    GameManager& game_manager_;
    int elo_range_;
    std::vector<WaitingPlayer> waiting_;
    int next_room_id_ = 1;
};
