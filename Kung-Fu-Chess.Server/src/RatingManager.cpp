#include "RatingManager.h"

#include <cmath>

namespace RatingManager {

namespace {

double expected_score(int rating, int opponent_rating) {
    return 1.0 / (1.0 + std::pow(10.0, (opponent_rating - rating) / 400.0));
}

} // namespace

std::pair<int, int> apply_result(int winner_rating, int loser_rating) {
    const double winner_expected = expected_score(winner_rating, loser_rating);
    const double loser_expected = expected_score(loser_rating, winner_rating);

    const int winner_delta = static_cast<int>(std::lround(kKFactor * (1.0 - winner_expected)));
    const int loser_delta = static_cast<int>(std::lround(kKFactor * (0.0 - loser_expected)));

    return { winner_rating + winner_delta, loser_rating + loser_delta };
}

} // namespace RatingManager
