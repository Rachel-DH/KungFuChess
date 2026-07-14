set(KFC_ENGINE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../Kung-Fu-Chess)

add_executable(Kung-Fu-Chess.Tests
    src/TestMain.cpp
    src/Board/BoardTests.cpp
    src/BoardMapper/BoardMapperTests.cpp
    src/Parser/ParseBoardTests.cpp
    src/Parser/BoardToStringTests.cpp
    src/Parser/RoundTripTests.cpp
    src/GameEngine/GameOverTests.cpp
    src/GameEngine/JumpTests.cpp
    src/GameEngine/PromotionTests.cpp
    src/GameEngine/WaitTests.cpp
    src/GameEngine/PrintTests.cpp
    src/Piece/KingTests.cpp
    src/Piece/QueenTests.cpp
    src/Piece/RookTests.cpp
    src/Piece/BishopTests.cpp
    src/Piece/KnightTests.cpp
    src/Piece/PawnTests.cpp
    src/Piece/PieceFactoryTests.cpp
    src/RuleEngine/RuleEngineTests.cpp
    src/Controller/ControllerTests.cpp
    src/CommandProcessor/CommandProcessorTests.cpp
    src/ProtocolIO/ProtocolIOTests.cpp
    ${KFC_ENGINE_DIR}/src/Board.cpp
    ${KFC_ENGINE_DIR}/src/Parser.cpp
    ${KFC_ENGINE_DIR}/src/GameEngine.cpp
    ${KFC_ENGINE_DIR}/src/Piece.cpp
    ${KFC_ENGINE_DIR}/src/RuleEngine.cpp
    ${KFC_ENGINE_DIR}/src/RealTimeArbiter.cpp
    ${KFC_ENGINE_DIR}/src/BoardMapper.cpp
    ${KFC_ENGINE_DIR}/src/Controller.cpp
    ${KFC_ENGINE_DIR}/src/CommandProcessor.cpp
    ${KFC_ENGINE_DIR}/src/ProtocolIO.cpp
)

target_include_directories(Kung-Fu-Chess.Tests PRIVATE
    include
    ${KFC_ENGINE_DIR}/include
)
