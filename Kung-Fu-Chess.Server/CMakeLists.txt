add_library(Kung-Fu-Chess.Server STATIC
    src/logics/model/Board.cpp
    src/logics/model/GameEngine.cpp
    src/logics/model/Piece.cpp
    src/logics/model/RuleEngine.cpp
    src/logics/control/RealTimeArbiter.cpp
    src/logics/control/Controller.cpp
    src/logics/input/Parser.cpp
)

# include/logics is added separately so existing headers keep resolving via
# their original unprefixed paths ("model/Board.h", "control/Controller.h",
# "input/Parser.h") without any #include edits; net/ code uses "net/X.h"
# resolved via the plain include root instead.
target_include_directories(Kung-Fu-Chess.Server PUBLIC
    include
    include/logics
)

target_link_libraries(Kung-Fu-Chess.Server PUBLIC Kung-Fu-Chess.Shared)

# websocketpp + standalone Asio, vendored in third_party/ rather than pulled
# through a package manager (architecture_plan.md §4.1/§4.6) -- proven working
# on this exact toolchain via Brachi's reference implementation, whose exact
# defines/include layout this mirrors.
set(KFC_THIRD_PARTY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../third_party)
target_include_directories(Kung-Fu-Chess.Server PUBLIC
    ${KFC_THIRD_PARTY_DIR}/asio/include
    ${KFC_THIRD_PARTY_DIR}/websocketpp
)
target_compile_definitions(Kung-Fu-Chess.Server PUBLIC
    ASIO_STANDALONE
    _WEBSOCKETPP_CPP11_STL_
    _WIN32_WINNT=0x0601
)
if(WIN32)
    target_link_libraries(Kung-Fu-Chess.Server PUBLIC ws2_32 mswsock)
endif()
