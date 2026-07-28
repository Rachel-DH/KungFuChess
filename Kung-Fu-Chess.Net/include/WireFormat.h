#pragma once

#include <stdexcept>
#include <string>

#include "net/Command.h"

// Isolated JSON <-> Command boundary (§4.1 of docs/architecture_plan.md).
// nlohmann::json never leaks past WireFormat.cpp; callers only see Command.
namespace WireFormat {

// Thrown on any malformed input; `code` is a short machine-readable tag
// (e.g. "UNKNOWN_COMMAND_TYPE", "MISSING_FIELD", "MALFORMED_JSON"),
// mirroring Parser::ParseError's convention.
class WireFormatError : public std::runtime_error {
public:
    explicit WireFormatError(const std::string& code) : std::runtime_error(code) {}
};

// Parses one inbound command message, e.g.
//   {"type":"move","start":{"x":0,"y":6},"dest":{"x":0,"y":4}}
//   {"type":"jump","cell":{"x":3,"y":3}}
// Throws WireFormatError on anything malformed or an unrecognized "type".
Command command_from_json(const std::string& json_text);

} // namespace WireFormat
