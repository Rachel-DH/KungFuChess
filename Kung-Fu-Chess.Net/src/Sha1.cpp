#include "Sha1.h"

extern "C" {
#include "ThirdParty/sha1.h"
}

namespace Sha1 {

std::array<unsigned char, 20> hash(const std::string& data) {
    SHA1_CTX ctx;
    std::array<unsigned char, 20> digest{};
    sha1_init(&ctx);
    sha1_update(&ctx, reinterpret_cast<const unsigned char*>(data.data()), data.size());
    sha1_final(&ctx, digest.data());
    return digest;
}

} // namespace Sha1
