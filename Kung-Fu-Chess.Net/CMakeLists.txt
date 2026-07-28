enable_language(C) # for the vendored sha1.c

set(KFC_ENGINE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../Kung-Fu-Chess)

add_library(Kung-Fu-Chess.Net STATIC
    src/WireFormat.cpp
    src/Base64.cpp
    src/WebSocketFrame.cpp
    src/ThirdParty/sha1.c
)

target_include_directories(Kung-Fu-Chess.Net PUBLIC
    include
    ${KFC_ENGINE_DIR}/include
)
target_include_directories(Kung-Fu-Chess.Net PRIVATE
    include/ThirdParty
)

set_source_files_properties(src/ThirdParty/sha1.c PROPERTIES COMPILE_OPTIONS "/W0")
