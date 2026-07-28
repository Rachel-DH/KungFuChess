#include "Matchmaker.h"

#include <algorithm>
#include <cmath>

Matchmaker::Matchmaker(GameManager& game_manager, int elo_range)
    : game_manager_(game_manager), elo_range_(elo_range) {
}

std::optional<std::string> Matchmaker::find_quick_match(const std::string& player_id, int rating) {
    auto it = std::find_if(waiting_.begin(), waiting_.end(), [&](const WaitingPlayer& waiting) {
        return std::abs(waiting.rating - rating) <= elo_range_;
    });
    if (it == waiting_.end()) {
        waiting_.push_back({ player_id, rating });
        return std::nullopt;
    }

    std::string opponent_id = it->player_id;
    waiting_.erase(it);

    std::string room_id = "quick-" + std::to_string(next_room_id_++);
    GameRoom& room = game_manager_.get_or_create(room_id);
    room.join(opponent_id); // the player who was already waiting becomes White
    room.join(player_id);   // the newly-arriving player becomes Black
    return room_id;
}

void Matchmaker::cancel_quick_match(const std::string& player_id) {
    waiting_.erase(
        std::remove_if(waiting_.begin(), waiting_.end(),
            [&](const WaitingPlayer& waiting) { return waiting.player_id == player_id; }),
        waiting_.end());
}
