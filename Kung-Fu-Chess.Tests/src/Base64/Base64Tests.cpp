#include "ThirdParty/doctest.h"

#include "Base64.h"

TEST_SUITE("Base64::encode") {

TEST_CASE("encodes a length that is a multiple of 3 with no padding") {
    const unsigned char data[] = { 'M', 'a', 'n' };
    CHECK(Base64::encode(data, 3) == "TWFu");
}

TEST_CASE("encodes a length one short of a multiple of 3 with double padding") {
    const unsigned char data[] = { 'M' };
    CHECK(Base64::encode(data, 1) == "TQ==");
}

TEST_CASE("encodes a length two short of a multiple of 3 with single padding") {
    const unsigned char data[] = { 'M', 'a' };
    CHECK(Base64::encode(data, 2) == "TWE=");
}

TEST_CASE("encoding zero bytes returns an empty string") {
    CHECK(Base64::encode(nullptr, 0) == "");
}

// The known test vector from RFC 6455's own handshake example.
TEST_CASE("matches the RFC 6455 handshake worked example") {
    const unsigned char sha1_digest[] = {
        0xb3, 0x7a, 0x4f, 0x2c, 0xc0, 0x62, 0x4f, 0x16,
        0x90, 0xf6, 0x46, 0x06, 0xcf, 0x38, 0x59, 0x45,
        0xb2, 0xbe, 0xc4, 0xea,
    };
    CHECK(Base64::encode(sha1_digest, sizeof(sha1_digest)) == "s3pPLMBiTxaQ9kYGzzhZRbK+xOo=");
}

}
