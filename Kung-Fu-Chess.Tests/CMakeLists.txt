set(KFC_LOCALPLAY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../Kung-Fu-Chess.LocalPlay)

add_executable(Kung-Fu-Chess.Tests
    src/TestMain.cpp
    src/Board/BoardTests.cpp
    src/BoardMapper/BoardMapperTests.cpp
    src/Parser/ParseBoardTests.cpp
    src/Parser/BoardToStringTests.cpp
    src/Parser/RoundTripTests.cpp
    src/GameEngine/GameOverTests.cpp
    src/GameEngine/HasActivityTests.cpp
    src/GameEngine/JumpTests.cpp
    src/GameEngine/PromotionTests.cpp
    src/GameEngine/RestTests.cpp
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
    src/RealTimeArbiter/RealTimeArbiterTests.cpp
    src/Controller/ControllerTests.cpp
    src/CommandProcessor/CommandProcessorTests.cpp
    src/ProtocolIO/ProtocolIOTests.cpp
    src/RenderLoop/RenderLoopTests.cpp
    src/AnimationStateConfig/AnimationStateConfigTests.cpp
    src/SpriteAnimation/SpriteAnimationTests.cpp
    src/PieceAnimationConfigProvider/PieceAnimationConfigProviderTests.cpp
    src/Networking/WebSocketRoundTripTests.cpp
    ${KFC_LOCALPLAY_DIR}/src/input/CommandProcessor.cpp
    ${KFC_LOCALPLAY_DIR}/src/input/ProtocolIO.cpp
    ${KFC_LOCALPLAY_DIR}/src/rendering/RenderLoop.cpp
)

# Kung-Fu-Chess.LocalPlay is an executable, not a library, so (same as
# before) its sources are recompiled directly here rather than linked.
# Everything else now comes from real library targets instead.
target_include_directories(Kung-Fu-Chess.Tests PRIVATE
    include
    ${KFC_LOCALPLAY_DIR}/include
)

target_link_libraries(Kung-Fu-Chess.Tests PRIVATE
    Kung-Fu-Chess.Server
    Kung-Fu-Chess.Client
    Kung-Fu-Chess.Shared
)

target_compile_definitions(Kung-Fu-Chess.Tests PRIVATE
    KFC_TEST_FIXTURES_DIR="${CMAKE_CURRENT_SOURCE_DIR}/fixtures"
)
