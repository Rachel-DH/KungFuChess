set(KFC_ENGINE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../Kung-Fu-Chess)

add_library(Kung-Fu-Chess.Net STATIC
    src/WireFormat.cpp
)

target_include_directories(Kung-Fu-Chess.Net PUBLIC
    include
    ${KFC_ENGINE_DIR}/include
)
