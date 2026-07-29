#pragma once

#include <stdexcept>
#include <string>

// One piece animation state's config, parsed from ext_src/pieces2/<code>/states/<state>/config.json's "physics"/"graphics" fields.
struct AnimationStateConfig {
    double speed_m_per_sec;
    std::string next_state_when_finished;
    int frames_per_sec;
    bool is_loop;
};

class AnimationConfigParseError : public std::runtime_error {
public:
    explicit AnimationConfigParseError(const std::string& what) : std::runtime_error(what) {}
};

// Pure parse over already-read text.
AnimationStateConfig parse_animation_state_config(const std::string& json_text);

// Reads the file at `path`, then parses it; throws AnimationConfigParseError if the file can't be opened or the content doesn't parse.
AnimationStateConfig read_animation_state_config(const std::string& path);
