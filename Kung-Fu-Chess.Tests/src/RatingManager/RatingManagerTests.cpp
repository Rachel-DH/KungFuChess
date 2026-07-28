#include "ThirdParty/doctest.h"

#include "RatingManager.h"

TEST_SUITE("RatingManager::apply_result") {

TEST_CASE("equally-rated players swing by exactly half the K-factor") {
    auto [new_winner, new_loser] = RatingManager::apply_result(1200, 1200);
    CHECK(new_winner == 1216);
    CHECK(new_loser == 1184);
}

TEST_CASE("beating a higher-rated opponent yields a larger gain than beating a lower-rated one") {
    auto [gain_vs_stronger, _1] = RatingManager::apply_result(1200, 1400);
    auto [gain_vs_weaker, _2] = RatingManager::apply_result(1200, 1000);

    CHECK((gain_vs_stronger - 1200) > (gain_vs_weaker - 1200));
}

TEST_CASE("the winner always gains rating and the loser always loses rating") {
    auto [new_winner, new_loser] = RatingManager::apply_result(1000, 1400);
    CHECK(new_winner > 1000);
    CHECK(new_loser < 1400);
}

TEST_CASE("a heavily-favored winner beating a much weaker opponent gains very little") {
    auto [new_winner, new_loser] = RatingManager::apply_result(1800, 1000);
    CHECK(new_winner - 1800 <= 1);
    CHECK(new_loser <= 1000);
}

}
