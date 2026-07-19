#include "ThirdParty/doctest.h"

#include "piece_animation_config_provider.h"

namespace {

std::string fixtures_pieces_dir() {
    return std::string(KFC_TEST_FIXTURES_DIR) + "/pieces2";
}

}  // namespace

TEST_SUITE("PieceAnimationConfigProvider::config_for") {
    TEST_CASE("a rook's idle config resolves to the real RW idle fixture") {
        PieceAnimationConfigProvider provider(fixtures_pieces_dir());

        const AnimationStateConfig& config = provider.config_for(PieceType::R, Color::w, PiecePhase::Idle);

        CHECK(config.speed_m_per_sec == 0.0);
        CHECK(config.next_state_when_finished == "idle");
        CHECK(config.frames_per_sec == 6);
        CHECK(config.is_loop == true);
    }

    TEST_CASE("the same rook's move config resolves to the real RW move fixture, distinct from its idle config") {
        PieceAnimationConfigProvider provider(fixtures_pieces_dir());

        const AnimationStateConfig& config = provider.config_for(PieceType::R, Color::w, PiecePhase::Move);

        CHECK(config.speed_m_per_sec == 1.5);
        CHECK(config.next_state_when_finished == "long_rest");
        CHECK(config.frames_per_sec == 12);
        CHECK(config.is_loop == true);
    }

    TEST_CASE("a black king's idle config resolves to the real KB idle fixture, not the rook's path") {
        PieceAnimationConfigProvider provider(fixtures_pieces_dir());

        const AnimationStateConfig& config = provider.config_for(PieceType::K, Color::b, PiecePhase::Idle);

        CHECK(config.speed_m_per_sec == 0.0);
        CHECK(config.next_state_when_finished == "idle");
        CHECK(config.frames_per_sec == 6);
        CHECK(config.is_loop == true);
    }

    TEST_CASE("a (type, color, phase) with no matching fixture file throws AnimationConfigParseError") {
        PieceAnimationConfigProvider provider(fixtures_pieces_dir());

        // No fixture exists for a white bishop under the test fixtures directory.
        CHECK_THROWS_AS(provider.config_for(PieceType::B, Color::w, PiecePhase::Idle), AnimationConfigParseError);
    }
}
