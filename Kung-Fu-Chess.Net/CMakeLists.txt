enable_language(C) # for the vendored sha1.c

set(KFC_ENGINE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../Kung-Fu-Chess)

# Kept as its own pure-C target (never mixed with this project's C++ files
# in one target) — MSVC's per-target /std:c++20 project setting otherwise
# leaks into the vendored .c file's compilation and breaks it.
add_library(Kung-Fu-Chess.Net.Sha1C STATIC
    src/ThirdParty/sha1.c
)
target_include_directories(Kung-Fu-Chess.Net.Sha1C PUBLIC include/ThirdParty)
set_source_files_properties(src/ThirdParty/sha1.c PROPERTIES COMPILE_OPTIONS "/W0")

add_library(Kung-Fu-Chess.Net STATIC
    src/WireFormat.cpp
    src/Base64.cpp
    src/WebSocketFrame.cpp
    src/Sha1.cpp
    src/WebSocketServer.cpp
    src/WsClient.cpp
)

target_link_libraries(Kung-Fu-Chess.Net PUBLIC Kung-Fu-Chess.Net.Sha1C)
if(WIN32)
    target_link_libraries(Kung-Fu-Chess.Net PUBLIC ws2_32)
endif()

target_include_directories(Kung-Fu-Chess.Net PUBLIC
    include
    ${KFC_ENGINE_DIR}/include
)
