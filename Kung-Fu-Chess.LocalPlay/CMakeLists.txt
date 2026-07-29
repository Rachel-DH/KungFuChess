add_executable(Kung-Fu-Chess.LocalPlay
    src/main.cpp
    src/input/CommandProcessor.cpp
    src/input/ProtocolIO.cpp
    src/rendering/RenderLoop.cpp
)

target_include_directories(Kung-Fu-Chess.LocalPlay PRIVATE include)
target_link_libraries(Kung-Fu-Chess.LocalPlay PRIVATE
    Kung-Fu-Chess.Server
    Kung-Fu-Chess.Client
    Kung-Fu-Chess.Shared
)

# Kung-Fu-Chess.Client pulls OpenCV in at link time, so this exe needs
# opencv_world*.dll beside it to run. Re-resolve OpenCV_LIB_PATH in this
# directory's scope (find_package's result in Kung-Fu-Chess.Client's scope
# doesn't propagate to this sibling directory).
find_package(OpenCV CONFIG REQUIRED)

if(WIN32 AND DEFINED OpenCV_LIB_PATH)
    # Vendored OpenCV Windows packs ship a single opencv_world<ver>[d].dll
    # instead of per-module DLLs; copy the one matching this build config
    # (Debug vs. Release) next to the exe so it runs without touching PATH.
    get_filename_component(_kfc_opencv_bin_dir "${OpenCV_LIB_PATH}/../bin" ABSOLUTE)
    file(GLOB _kfc_opencv_world_release "${_kfc_opencv_bin_dir}/opencv_world[0-9][0-9][0-9].dll")
    file(GLOB _kfc_opencv_world_debug "${_kfc_opencv_bin_dir}/opencv_world[0-9][0-9][0-9]d.dll")
    if(_kfc_opencv_world_release AND _kfc_opencv_world_debug)
        add_custom_command(TARGET Kung-Fu-Chess.LocalPlay POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "$<IF:$<CONFIG:Debug>,${_kfc_opencv_world_debug},${_kfc_opencv_world_release}>"
                "$<TARGET_FILE_DIR:Kung-Fu-Chess.LocalPlay>"
        )
    endif()
    unset(_kfc_opencv_world_release)
    unset(_kfc_opencv_world_debug)
    unset(_kfc_opencv_bin_dir)
endif()
