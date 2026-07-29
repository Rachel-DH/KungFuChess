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
# "input/Parser.h") without any #include edits; new net/ code added later
# uses "net/X.h" resolved via the plain include root instead.
target_include_directories(Kung-Fu-Chess.Server PUBLIC
    include
    include/logics
)

target_link_libraries(Kung-Fu-Chess.Server PUBLIC Kung-Fu-Chess.Shared)
