#include "ThirdParty/doctest.h"

#include "WebSocketFrame.h"

TEST_SUITE("WebSocketFrame::encode/decode round trip") {

TEST_CASE("an unmasked server text frame round-trips through try_decode_frame") {
    auto bytes = WebSocketFrame::encode_server_text_frame("hello");
    std::vector<unsigned char> buffer(bytes.begin(), bytes.end());

    auto frame = WebSocketFrame::try_decode_frame(buffer);
    REQUIRE(frame.has_value());
    CHECK(frame->opcode == WebSocketFrame::kOpcodeText);
    CHECK(frame->payload == "hello");
    CHECK(buffer.empty()); // the consumed frame's bytes are erased
}

TEST_CASE("a masked client text frame round-trips through try_decode_frame") {
    std::array<unsigned char, 4> mask_key{ 0x12, 0x34, 0x56, 0x78 };
    auto bytes = WebSocketFrame::encode_client_text_frame("hi there", mask_key);
    std::vector<unsigned char> buffer(bytes.begin(), bytes.end());

    auto frame = WebSocketFrame::try_decode_frame(buffer);
    REQUIRE(frame.has_value());
    CHECK(frame->payload == "hi there");
}

TEST_CASE("a payload longer than 125 bytes uses the 16-bit extended length and still round-trips") {
    std::string payload(500, 'x');
    auto bytes = WebSocketFrame::encode_server_text_frame(payload);
    std::vector<unsigned char> buffer(bytes.begin(), bytes.end());

    auto frame = WebSocketFrame::try_decode_frame(buffer);
    REQUIRE(frame.has_value());
    CHECK(frame->payload == payload);
}

TEST_CASE("an empty payload round-trips") {
    auto bytes = WebSocketFrame::encode_server_text_frame("");
    std::vector<unsigned char> buffer(bytes.begin(), bytes.end());

    auto frame = WebSocketFrame::try_decode_frame(buffer);
    REQUIRE(frame.has_value());
    CHECK(frame->payload == "");
}

}

TEST_SUITE("WebSocketFrame::try_decode_frame — partial buffers") {

TEST_CASE("an empty buffer returns nullopt") {
    std::vector<unsigned char> buffer;
    CHECK_FALSE(WebSocketFrame::try_decode_frame(buffer).has_value());
}

TEST_CASE("a frame whose payload hasn't fully arrived yet returns nullopt and leaves the buffer untouched") {
    auto bytes = WebSocketFrame::encode_server_text_frame("hello world");
    std::vector<unsigned char> full(bytes.begin(), bytes.end());
    std::vector<unsigned char> partial(full.begin(), full.end() - 3); // truncate the tail

    CHECK_FALSE(WebSocketFrame::try_decode_frame(partial).has_value());
    CHECK(partial.size() == full.size() - 3); // untouched, nothing erased
}

TEST_CASE("two frames queued back-to-back decode one at a time in order") {
    auto first = WebSocketFrame::encode_server_text_frame("first");
    auto second = WebSocketFrame::encode_server_text_frame("second");
    std::vector<unsigned char> buffer(first.begin(), first.end());
    buffer.insert(buffer.end(), second.begin(), second.end());

    auto decoded_first = WebSocketFrame::try_decode_frame(buffer);
    REQUIRE(decoded_first.has_value());
    CHECK(decoded_first->payload == "first");

    auto decoded_second = WebSocketFrame::try_decode_frame(buffer);
    REQUIRE(decoded_second.has_value());
    CHECK(decoded_second->payload == "second");
    CHECK(buffer.empty());
}

}
