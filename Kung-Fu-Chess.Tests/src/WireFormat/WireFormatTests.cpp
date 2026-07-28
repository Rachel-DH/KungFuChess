#include "ThirdParty/doctest.h"

#include "WireFormat.h"

TEST_SUITE("WireFormat::command_from_json") {

TEST_CASE("a well-formed MOVE JSON parses to the matching MoveCommand") {
    Command command = WireFormat::command_from_json(
        R"({"type":"move","start":{"x":0,"y":6},"dest":{"x":0,"y":4}})");
    REQUIRE(std::holds_alternative<MoveCommand>(command));
    const auto& move = std::get<MoveCommand>(command);
    CHECK(move.start == Position{ 0, 6 });
    CHECK(move.dest == Position{ 0, 4 });
}

TEST_CASE("a well-formed JUMP JSON parses to the matching JumpCommand") {
    Command command = WireFormat::command_from_json(R"({"type":"jump","cell":{"x":3,"y":3}})");
    REQUIRE(std::holds_alternative<JumpCommand>(command));
    CHECK(std::get<JumpCommand>(command).cell == Position{ 3, 3 });
}

TEST_CASE("an unrecognized type value throws WireFormatError") {
    CHECK_THROWS_AS(
        WireFormat::command_from_json(R"({"type":"teleport","start":{"x":0,"y":0},"dest":{"x":1,"y":1}})"),
        WireFormat::WireFormatError);
}

TEST_CASE("a MOVE JSON missing the dest field throws WireFormatError") {
    CHECK_THROWS_AS(
        WireFormat::command_from_json(R"({"type":"move","start":{"x":0,"y":6}})"),
        WireFormat::WireFormatError);
}

TEST_CASE("a MOVE JSON with a non-integer coordinate throws WireFormatError") {
    CHECK_THROWS_AS(
        WireFormat::command_from_json(R"({"type":"move","start":{"x":"a","y":6},"dest":{"x":0,"y":4}})"),
        WireFormat::WireFormatError);
}

TEST_CASE("text that isn't a valid JSON object throws WireFormatError") {
    SUBCASE("not json at all") {
        CHECK_THROWS_AS(WireFormat::command_from_json("not json"), WireFormat::WireFormatError);
    }
    SUBCASE("a bare JSON array") {
        CHECK_THROWS_AS(WireFormat::command_from_json("[1,2,3]"), WireFormat::WireFormatError);
    }
}

}

TEST_SUITE("WireFormat::client_message_from_json") {

TEST_CASE("a well-formed register message parses to RegisterMessage") {
    auto message = WireFormat::client_message_from_json(R"({"type":"register","username":"alice","password":"hunter2"})");
    REQUIRE(std::holds_alternative<RegisterMessage>(message));
    CHECK(std::get<RegisterMessage>(message).username == "alice");
    CHECK(std::get<RegisterMessage>(message).password == "hunter2");
}

TEST_CASE("a well-formed login message parses to LoginMessage") {
    auto message = WireFormat::client_message_from_json(R"({"type":"login","username":"alice","password":"hunter2"})");
    REQUIRE(std::holds_alternative<LoginMessage>(message));
    CHECK(std::get<LoginMessage>(message).username == "alice");
}

TEST_CASE("a well-formed join_room message parses to JoinRoomMessage") {
    auto message = WireFormat::client_message_from_json(R"({"type":"join_room","room_name":"my-room"})");
    REQUIRE(std::holds_alternative<JoinRoomMessage>(message));
    CHECK(std::get<JoinRoomMessage>(message).room_name == "my-room");
}

TEST_CASE("a quick_match message parses to QuickMatchMessage") {
    auto message = WireFormat::client_message_from_json(R"({"type":"quick_match"})");
    CHECK(std::holds_alternative<QuickMatchMessage>(message));
}

TEST_CASE("a resign message parses to ResignMessage") {
    auto message = WireFormat::client_message_from_json(R"({"type":"resign"})");
    CHECK(std::holds_alternative<ResignMessage>(message));
}

TEST_CASE("a move message parses to MoveCommand") {
    auto message = WireFormat::client_message_from_json(
        R"({"type":"move","start":{"x":0,"y":6},"dest":{"x":0,"y":4}})");
    REQUIRE(std::holds_alternative<MoveCommand>(message));
}

TEST_CASE("a register message missing the password field throws WireFormatError") {
    CHECK_THROWS_AS(
        WireFormat::client_message_from_json(R"({"type":"register","username":"alice"})"),
        WireFormat::WireFormatError);
}

TEST_CASE("an unrecognized type throws WireFormatError") {
    CHECK_THROWS_AS(WireFormat::client_message_from_json(R"({"type":"nonsense"})"), WireFormat::WireFormatError);
}

}
