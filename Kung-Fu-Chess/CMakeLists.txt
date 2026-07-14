add_executable(Kung-Fu-Chess
    src/main.cpp
    src/Board.cpp
    src/Parser.cpp
    src/GameEngine.cpp
    src/Piece.cpp
    src/RuleEngine.cpp
    src/RealTimeArbiter.cpp
    src/BoardMapper.cpp
    src/Controller.cpp
    src/CommandProcessor.cpp
    src/ProtocolIO.cpp
)

target_include_directories(Kung-Fu-Chess PRIVATE include)
