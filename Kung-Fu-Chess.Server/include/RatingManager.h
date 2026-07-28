#pragma once

#include <utility>

// Standard ELO (§5): every user starts at kStartingRating; a decisive result
// (this variant has no draws — a king capture always ends the game) updates
// both ratings proportionally to how surprising the result was.
namespace RatingManager {

constexpr int kStartingRating = 1200;
constexpr int kKFactor = 32;

// Returns {new_winner_rating, new_loser_rating} after winner_rating beats loser_rating.
std::pair<int, int> apply_result(int winner_rating, int loser_rating);

} // namespace RatingManager
