#include "animation_state_config.h"

#include <cctype>
#include <fstream>
#include <sstream>

namespace {

// Locates `"key"` in `text`, then returns the offset of the first non-whitespace character after its colon; throws naming `key` if it's missing, has no colon, or has no value.
std::size_t value_start_after_key(const std::string& text, const std::string& key) {
    std::string needle = "\"" + key + "\"";
    std::size_t key_pos = text.find(needle);
    if (key_pos == std::string::npos) {
        throw AnimationConfigParseError("Missing key: " + key);
    }

    std::size_t colon_pos = text.find(':', key_pos + needle.size());
    if (colon_pos == std::string::npos) {
        throw AnimationConfigParseError("Missing ':' after key: " + key);
    }

    std::size_t value_pos = colon_pos + 1;
    while (value_pos < text.size() && std::isspace(static_cast<unsigned char>(text[value_pos]))) {
        ++value_pos;
    }
    if (value_pos >= text.size()) {
        throw AnimationConfigParseError("Missing value for key: " + key);
    }
    return value_pos;
}

double parse_double_value(const std::string& text, const std::string& key) {
    std::size_t start = value_start_after_key(text, key);
    try {
        return std::stod(text.substr(start));
    } catch (const std::exception&) {
        throw AnimationConfigParseError("Invalid numeric value for key: " + key);
    }
}

int parse_int_value(const std::string& text, const std::string& key) {
    std::size_t start = value_start_after_key(text, key);
    try {
        return std::stoi(text.substr(start));
    } catch (const std::exception&) {
        throw AnimationConfigParseError("Invalid integer value for key: " + key);
    }
}

std::string parse_string_value(const std::string& text, const std::string& key) {
    std::size_t start = value_start_after_key(text, key);
    if (text[start] != '"') {
        throw AnimationConfigParseError("Expected quoted string for key: " + key);
    }
    std::size_t end = text.find('"', start + 1);
    if (end == std::string::npos) {
        throw AnimationConfigParseError("Unterminated string value for key: " + key);
    }
    return text.substr(start + 1, end - start - 1);
}

bool parse_bool_value(const std::string& text, const std::string& key) {
    std::size_t start = value_start_after_key(text, key);
    if (text.compare(start, 4, "true") == 0) {
        return true;
    }
    if (text.compare(start, 5, "false") == 0) {
        return false;
    }
    throw AnimationConfigParseError("Invalid boolean value for key: " + key);
}

}  // namespace

AnimationStateConfig parse_animation_state_config(const std::string& json_text) {
    AnimationStateConfig config;
    config.speed_m_per_sec = parse_double_value(json_text, "speed_m_per_sec");
    config.next_state_when_finished = parse_string_value(json_text, "next_state_when_finished");
    config.frames_per_sec = parse_int_value(json_text, "frames_per_sec");
    config.is_loop = parse_bool_value(json_text, "is_loop");
    return config;
}

AnimationStateConfig read_animation_state_config(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        throw AnimationConfigParseError("Cannot open file: " + path);
    }

    std::ostringstream buffer;
    buffer << file.rdbuf();
    return parse_animation_state_config(buffer.str());
}
