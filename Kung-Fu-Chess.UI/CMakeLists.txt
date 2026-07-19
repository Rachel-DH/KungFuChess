find_package(OpenCV CONFIG REQUIRED)

set(KFC_ENGINE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../Kung-Fu-Chess)

add_library(Kung-Fu-Chess.UI STATIC
    src/asset_path.cpp
    src/img.cpp
    src/renderer.cpp
    src/renderer_factory.cpp
    src/opencv_input_source.cpp
    src/input_source_factory.cpp
    src/animation_state_config.cpp
    src/sprite_animation.cpp
    src/piece_animation_config_provider.cpp
    src/piece_naming.cpp
    src/sprite_frame_counter.cpp
    src/sprite_manager.cpp
)

target_include_directories(Kung-Fu-Chess.UI PUBLIC
    include
    ${KFC_ENGINE_DIR}/include
)
target_link_libraries(Kung-Fu-Chess.UI PUBLIC ${OpenCV_LIBS})
