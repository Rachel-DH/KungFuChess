set(KFC_ENGINE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../Kung-Fu-Chess)

add_executable(Kung-Fu-Chess.Tests
    src/TestMain.cpp
    src/Board/BoardTests.cpp
    src/BoardMapper/BoardMapperTests.cpp
    src/Parser/ParseBoardTests.cpp
    src/Parser/BoardToStringTests.cpp
    src/Parser/RoundTripTests.cpp
    src/GameEngine/GameOverTests.cpp
    src/GameEngine/ForceGameOverTests.cpp
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
    src/CommandDispatcher/CommandDispatcherTests.cpp
    src/WireFormat/WireFormatTests.cpp
    src/EventBus/EventBusTests.cpp
    src/RatingManager/RatingManagerTests.cpp
    src/GameRoom/GameRoomTests.cpp
    src/GameManager/GameManagerTests.cpp
    src/Matchmaker/MatchmakerTests.cpp
    src/PasswordHasher/PasswordHasherTests.cpp
    src/InMemoryUserRepository/InMemoryUserRepositoryTests.cpp
    src/SqliteUserRepository/SqliteUserRepositoryTests.cpp
    src/Base64/Base64Tests.cpp
    src/WebSocketFrame/WebSocketFrameTests.cpp
    src/WebSocketServer/WebSocketServerTests.cpp
    src/ConnectionManager/ConnectionManagerTests.cpp
    src/AnimationStateConfig/AnimationStateConfigTests.cpp
    src/SpriteAnimation/SpriteAnimationTests.cpp
    src/PieceAnimationConfigProvider/PieceAnimationConfigProviderTests.cpp
    ${KFC_ENGINE_DIR}/src/model/Board.cpp
    ${KFC_ENGINE_DIR}/src/input/Parser.cpp
    ${KFC_ENGINE_DIR}/src/model/GameEngine.cpp
    ${KFC_ENGINE_DIR}/src/model/Piece.cpp
    ${KFC_ENGINE_DIR}/src/model/RuleEngine.cpp
    ${KFC_ENGINE_DIR}/src/control/RealTimeArbiter.cpp
    ${KFC_ENGINE_DIR}/src/control/BoardMapper.cpp
    ${KFC_ENGINE_DIR}/src/control/Controller.cpp
    ${KFC_ENGINE_DIR}/src/input/CommandProcessor.cpp
    ${KFC_ENGINE_DIR}/src/input/ProtocolIO.cpp
    ${KFC_ENGINE_DIR}/src/rendering/RenderLoop.cpp
    ${KFC_ENGINE_DIR}/src/net/CommandDispatcher.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../Kung-Fu-Chess.UI/src/animation/animation_state_config.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../Kung-Fu-Chess.UI/src/animation/sprite_animation.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../Kung-Fu-Chess.UI/src/animation/piece_animation_config_provider.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../Kung-Fu-Chess.UI/src/animation/piece_naming.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../Kung-Fu-Chess.UI/src/assets/asset_path.cpp
)

target_include_directories(Kung-Fu-Chess.Tests PRIVATE
    include
    ${KFC_ENGINE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../Kung-Fu-Chess.UI/include
)

target_link_libraries(Kung-Fu-Chess.Tests PRIVATE Kung-Fu-Chess.Net Kung-Fu-Chess.Server)

target_compile_definitions(Kung-Fu-Chess.Tests PRIVATE
    KFC_TEST_FIXTURES_DIR="${CMAKE_CURRENT_SOURCE_DIR}/fixtures"
)
