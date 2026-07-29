#include "wire/WireFormat.h"

void to_json(nlohmann::json& j, const Position& pos) {
    j = nlohmann::json{ { "x", pos.x }, { "y", pos.y } };
}

void from_json(const nlohmann::json& j, Position& pos) {
    j.at("x").get_to(pos.x);
    j.at("y").get_to(pos.y);
}
