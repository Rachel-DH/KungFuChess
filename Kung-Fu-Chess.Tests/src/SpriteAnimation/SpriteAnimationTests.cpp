#include "ThirdParty/doctest.h"

#include "sprite_animation.h"

TEST_SUITE("SpriteAnimation::advance") {
    TEST_CASE("a fresh animation starts at frame 0") {
        SpriteAnimation animation(5, 12, true);

        CHECK(animation.frame_index() == 0);
    }

    TEST_CASE("advancing less than one frame's worth doesn't move it") {
        SpriteAnimation animation(5, 12, true);  // ~83ms per frame

        animation.advance(50);

        CHECK(animation.frame_index() == 0);
    }

    TEST_CASE("advancing exactly one frame's worth moves to the next") {
        SpriteAnimation animation(5, 12, true);  // ~83ms per frame

        animation.advance(83);

        CHECK(animation.frame_index() == 1);
    }

    TEST_CASE("split sub-frame calls don't lose leftover time") {
        SpriteAnimation animation(5, 12, true);  // ~83ms per frame

        animation.advance(50);
        animation.advance(50);

        CHECK(animation.frame_index() == 1);
    }

    TEST_CASE("a looping animation wraps to frame 0 after its last frame, never finishes") {
        SpriteAnimation animation(5, 12, true);

        animation.advance(5 * 83);

        CHECK(animation.frame_index() == 0);
        CHECK(animation.finished() == false);
    }

    TEST_CASE("a non-looping animation clamps at the last frame, reports finished, and further advance is a no-op") {
        SpriteAnimation animation(5, 12, false);

        animation.advance(5 * 83);

        CHECK(animation.frame_index() == 4);
        CHECK(animation.finished() == true);

        animation.advance(1000);

        CHECK(animation.frame_index() == 4);
        CHECK(animation.finished() == true);
    }
}

TEST_SUITE("SpriteAnimation::reset") {
    TEST_CASE("reset returns to frame 0 and clears finished") {
        SpriteAnimation animation(5, 12, false);
        animation.advance(5 * 83);
        REQUIRE(animation.finished() == true);

        animation.reset();

        CHECK(animation.frame_index() == 0);
        CHECK(animation.finished() == false);
    }
}

TEST_SUITE("SpriteAnimation::from_config") {
    TEST_CASE("built from real move values, advancing one frame's worth moves to the next") {
        AnimationStateConfig move_config{ 1.5, "long_rest", 12, true };

        SpriteAnimation animation = SpriteAnimation::from_config(move_config, 5);
        animation.advance(83);

        CHECK(animation.frame_index() == 1);
    }

    TEST_CASE("built from real jump values, advancing through all frames finishes on the last one") {
        AnimationStateConfig jump_config{ 3.0, "short_rest", 8, false };  // 125ms per frame

        SpriteAnimation animation = SpriteAnimation::from_config(jump_config, 5);
        animation.advance(5 * 125);

        CHECK(animation.frame_index() == 4);
        CHECK(animation.finished() == true);
    }
}
