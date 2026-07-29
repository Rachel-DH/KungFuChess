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
)

target_include_directories(Kung-Fu-Chess.Client PUBLIC include)
target_link_libraries(Kung-Fu-Chess.Client PUBLIC ${OpenCV_LIBS} Kung-Fu-Chess.Shared)
