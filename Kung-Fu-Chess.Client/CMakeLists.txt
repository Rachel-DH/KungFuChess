find_package(OpenCV CONFIG REQUIRED)

add_library(Kung-Fu-Chess.Client STATIC
    src/assets/asset_path.cpp
    src/assets/img.cpp
    src/rendering/renderer.cpp
    src/rendering/renderer_factory.cpp
    src/input/opencv_input_source.cpp
    src/input/input_source_factory.cpp
    src/animation/animation_state_config.cpp
    src/animation/sprite_animation.cpp
    src/animation/piece_animation_config_provider.cpp
    src/animation/piece_naming.cpp
    src/animation/sprite_frame_counter.cpp
    src/animation/sprite_manager.cpp
    src/net/WsClient.cpp
)

target_include_directories(Kung-Fu-Chess.Client PUBLIC include)
target_link_libraries(Kung-Fu-Chess.Client PUBLIC ${OpenCV_LIBS} Kung-Fu-Chess.Shared)

# websocketpp + standalone Asio, vendored in third_party/ -- same rationale
# and exact defines as Kung-Fu-Chess.Server (see its CMakeLists.txt).
set(KFC_THIRD_PARTY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../third_party)
target_include_directories(Kung-Fu-Chess.Client PUBLIC
    ${KFC_THIRD_PARTY_DIR}/asio/include
    ${KFC_THIRD_PARTY_DIR}/websocketpp
)
target_compile_definitions(Kung-Fu-Chess.Client PUBLIC
    ASIO_STANDALONE
    _WEBSOCKETPP_CPP11_STL_
    _WIN32_WINNT=0x0601
)
if(WIN32)
    target_link_libraries(Kung-Fu-Chess.Client PUBLIC ws2_32 mswsock)
endif()
