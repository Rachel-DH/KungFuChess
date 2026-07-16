cmake_minimum_required(VERSION 3.20)

option(KFC_BUILD_UI "Build Kung-Fu-Chess.UI (requires a vendored OpenCV under deps/opencv)" ON)

# Pick up a vendored OpenCV package (extracted straight from the opencv.org
# Windows release) automatically, if present. Only Kung-Fu-Chess.UI needs it
# — the core engine (Kung-Fu-Chess) stays free of any OpenCV dependency.
if(KFC_BUILD_UI AND NOT DEFINED OpenCV_DIR)
    set(_kfc_opencv_dir "${CMAKE_CURRENT_SOURCE_DIR}/deps/opencv/build")
    if(EXISTS "${_kfc_opencv_dir}/OpenCVConfig.cmake")
        set(OpenCV_DIR "${_kfc_opencv_dir}" CACHE PATH "")
        # The Windows pack only ships vc16 (VS2019) binaries, and its compiler
        # detection doesn't know about newer MSVC point releases (e.g. recent
        # VS2022 updates). The vc16 binaries are ABI-compatible, so pin the
        # runtime explicitly instead of letting detection fail.
        set(OpenCV_ARCH x64 CACHE STRING "")
        set(OpenCV_RUNTIME vc16 CACHE STRING "")
    endif()
    unset(_kfc_opencv_dir)
endif()

project(KungFuChess CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(Kung-Fu-Chess)
add_subdirectory(Kung-Fu-Chess.Tests)

if(KFC_BUILD_UI)
    add_subdirectory(Kung-Fu-Chess.UI)
endif()
