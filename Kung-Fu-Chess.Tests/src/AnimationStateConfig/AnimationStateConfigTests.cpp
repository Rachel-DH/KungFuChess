#include "ThirdParty/doctest.h"

#include "animation_state_config.h"

TEST_SUITE("AnimationStateConfig::parse") {
    TEST_CASE("a real idle config parses correctly") {
        std::string json_text = R"({
  "physics": {
    "speed_m_per_sec": 0.0,
    "next_state_when_finished": "idle"
  },
  "graphics": {
    "frames_per_sec": 6,
    "is_loop": true
  }
})";

        AnimationStateConfig config = parse_animation_state_config(json_text);

        CHECK(config.speed_m_per_sec == 0.0);
        CHECK(config.next_state_when_finished == "idle");
        CHECK(config.frames_per_sec == 6);
        CHECK(config.is_loop == true);
    }

    TEST_CASE("a real move config parses correctly") {
        std::string json_text = R"({
  "physics": {
    "speed_m_per_sec": 1.5,
    "next_state_when_finished": "long_rest"
  },
  "graphics": {
    "frames_per_sec": 12,
    "is_loop": true
  }
})";

        AnimationStateConfig config = parse_animation_state_config(json_text);

        CHECK(config.speed_m_per_sec == 1.5);
        CHECK(config.next_state_when_finished == "long_rest");
        CHECK(config.frames_per_sec == 12);
        CHECK(config.is_loop == true);
    }

    TEST_CASE("a real jump config parses correctly") {
        std::string json_text = R"({
  "physics": {
    "speed_m_per_sec": 3.0,
    "next_state_when_finished": "short_rest"
  },
  "graphics": {
    "frames_per_sec": 8,
    "is_loop": false
  }
})";

        AnimationStateConfig config = parse_animation_state_config(json_text);

        CHECK(config.speed_m_per_sec == 3.0);
        CHECK(config.next_state_when_finished == "short_rest");
        CHECK(config.frames_per_sec == 8);
        CHECK(config.is_loop == false);
    }

    TEST_CASE("the same move values as compact single-line JSON parse identically") {
        std::string json_text =
            R"({"physics":{"speed_m_per_sec":1.5,"next_state_when_finished":"long_rest"},)"
            R"("graphics":{"frames_per_sec":12,"is_loop":true}})";

        AnimationStateConfig config = parse_animation_state_config(json_text);

        CHECK(config.speed_m_per_sec == 1.5);
        CHECK(config.next_state_when_finished == "long_rest");
        CHECK(config.frames_per_sec == 12);
        CHECK(config.is_loop == true);
    }

    TEST_CASE("a config missing a required key throws AnimationConfigParseError") {
        std::string json_text = R"({
  "physics": {
    "speed_m_per_sec": 1.5,
    "next_state_when_finished": "long_rest"
  },
  "graphics": {
    "frames_per_sec": 12
  }
})";

        CHECK_THROWS_AS(parse_animation_state_config(json_text), AnimationConfigParseError);
    }
}
