cmake_minimum_required(VERSION 3.20)

# Pick up a vendored OpenCV package (extracted straight from the opencv.org
# Windows release) automatically, if present. Kung-Fu-Chess.Client needs it
# (and Kung-Fu-Chess.Tests links against Kung-Fu-Chess.Client, so it needs
# OpenCV available too, unconditionally).
if(NOT DEFINED OpenCV_DIR)
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

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(KFC_BUILD_LOCALPLAY "Build the local single-process windowed executable (no networking; requires vendored OpenCV via Kung-Fu-Chess.Client, which is always built since Kung-Fu-Chess.Tests needs it too)" ON)

add_subdirectory(Kung-Fu-Chess.Shared)
add_subdirectory(Kung-Fu-Chess.Server)
add_subdirectory(Kung-Fu-Chess.Client)
if(KFC_BUILD_LOCALPLAY)
    add_subdirectory(Kung-Fu-Chess.LocalPlay)
endif()
add_subdirectory(Kung-Fu-Chess.Tests)
