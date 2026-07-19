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
    src/RenderLoop/RenderLoopTests.cpp
    src/AnimationStateConfig/AnimationStateConfigTests.cpp
    src/SpriteAnimation/SpriteAnimationTests.cpp
    src/PieceAnimationConfigProvider/PieceAnimationConfigProviderTests.cpp
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
    ${KFC_ENGINE_DIR}/src/RenderLoop.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../Kung-Fu-Chess.UI/src/animation_state_config.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../Kung-Fu-Chess.UI/src/sprite_animation.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../Kung-Fu-Chess.UI/src/piece_animation_config_provider.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../Kung-Fu-Chess.UI/src/piece_naming.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../Kung-Fu-Chess.UI/src/asset_path.cpp
)

target_include_directories(Kung-Fu-Chess.Tests PRIVATE
    include
    ${KFC_ENGINE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../Kung-Fu-Chess.UI/include
)

target_compile_definitions(Kung-Fu-Chess.Tests PRIVATE
    KFC_TEST_FIXTURES_DIR="${CMAKE_CURRENT_SOURCE_DIR}/fixtures"
)
