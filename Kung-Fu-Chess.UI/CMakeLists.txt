find_package(OpenCV CONFIG REQUIRED)

set(KFC_ENGINE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../Kung-Fu-Chess)

add_library(Kung-Fu-Chess.UI STATIC
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
)

target_include_directories(Kung-Fu-Chess.UI PUBLIC
    include
    ${KFC_ENGINE_DIR}/include
)
target_link_libraries(Kung-Fu-Chess.UI PUBLIC ${OpenCV_LIBS})
