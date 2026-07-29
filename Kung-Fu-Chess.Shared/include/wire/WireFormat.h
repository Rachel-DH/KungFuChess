#pragma once

#include <nlohmann/json.hpp>

#include "model/Position.h"

// Wire encoding for Position: {"x": <int>, "y": <int>}. Free functions in the global
// namespace so nlohmann's ADL finds them without a wrapper type. No bounds validation
// here - this is pure marshalling; nlohmann's own exceptions propagate on malformed input.
void to_json(nlohmann::json& j, const Position& pos);
void from_json(const nlohmann::json& j, Position& pos);
