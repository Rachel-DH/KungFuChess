#include "WireFormat.h"

#include "ThirdParty/nlohmann/json.hpp"

namespace WireFormat {

namespace {

using nlohmann::json;

Position position_from_json(const json& node) {
    if (!node.is_object() || !node.contains("x") || !node.contains("y")
        || !node["x"].is_number_integer() || !node["y"].is_number_integer()) {
        throw WireFormatError("MISSING_FIELD");
    }
    return Position{ node["x"].get<int>(), node["y"].get<int>() };
}

} // namespace

Command command_from_json(const std::string& json_text) {
    json root;
    try {
        root = json::parse(json_text);
    } catch (const json::parse_error&) {
        throw WireFormatError("MALFORMED_JSON");
    }

    if (!root.is_object() || !root.contains("type") || !root["type"].is_string()) {
        throw WireFormatError("MALFORMED_JSON");
    }

    const std::string type = root["type"].get<std::string>();
    if (type == "move") {
        if (!root.contains("start") || !root.contains("dest")) {
            throw WireFormatError("MISSING_FIELD");
        }
        return MoveCommand{ position_from_json(root["start"]), position_from_json(root["dest"]) };
    }
    if (type == "jump") {
        if (!root.contains("cell")) {
            throw WireFormatError("MISSING_FIELD");
        }
        return JumpCommand{ position_from_json(root["cell"]) };
    }
    throw WireFormatError("UNKNOWN_COMMAND_TYPE");
}

} // namespace WireFormat
