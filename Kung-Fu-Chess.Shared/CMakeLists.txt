add_library(Kung-Fu-Chess.Shared STATIC
    src/control/BoardMapper.cpp
    src/wire/WireFormat.cpp
)

target_include_directories(Kung-Fu-Chess.Shared PUBLIC include)

# nlohmann::json, vendored in third_party/ rather than pulled through a
# package manager (same rationale as asio/websocketpp in Server/Client).
# PUBLIC because Server and Client both consume Shared and need json.hpp
# transitively for the wire format.
set(KFC_THIRD_PARTY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../third_party)
target_include_directories(Kung-Fu-Chess.Shared PUBLIC
    ${KFC_THIRD_PARTY_DIR}/nlohmann_json/single_include
)
