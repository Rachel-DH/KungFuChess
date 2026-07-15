find_package(OpenCV CONFIG REQUIRED)

add_executable(Kung-Fu-Chess.UI
    src/main.cpp
    src/img.cpp
)

target_include_directories(Kung-Fu-Chess.UI PRIVATE include)
target_link_libraries(Kung-Fu-Chess.UI PRIVATE ${OpenCV_LIBS})

if(WIN32 AND DEFINED OpenCV_LIB_PATH)
    # Vendored OpenCV Windows packs ship a single opencv_world<ver>[d].dll
    # instead of per-module DLLs; copy the one matching this build config
    # (Debug vs. Release) next to the exe so it runs without touching PATH.
    get_filename_component(_kfc_opencv_bin_dir "${OpenCV_LIB_PATH}/../bin" ABSOLUTE)
    file(GLOB _kfc_opencv_world_release "${_kfc_opencv_bin_dir}/opencv_world[0-9][0-9][0-9].dll")
    file(GLOB _kfc_opencv_world_debug "${_kfc_opencv_bin_dir}/opencv_world[0-9][0-9][0-9]d.dll")
    if(_kfc_opencv_world_release AND _kfc_opencv_world_debug)
        add_custom_command(TARGET Kung-Fu-Chess.UI POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "$<IF:$<CONFIG:Debug>,${_kfc_opencv_world_debug},${_kfc_opencv_world_release}>"
                "$<TARGET_FILE_DIR:Kung-Fu-Chess.UI>"
        )
    endif()
    unset(_kfc_opencv_world_release)
    unset(_kfc_opencv_world_debug)
    unset(_kfc_opencv_bin_dir)
endif()