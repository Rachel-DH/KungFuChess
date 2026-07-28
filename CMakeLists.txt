cmake_minimum_required(VERSION 3.20)

# Pick up a vendored OpenCV package (extracted straight from the opencv.org
# Windows release) automatically, if present. Only Kung-Fu-Chess.UI needs it
# — Kung-Fu-Chess.Tests recompiles the engine sources directly and stays
# free of any OpenCV dependency.
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

# Kung-Fu-Chess.UI's find_package(OpenCV ... REQUIRED) fails at configure
# time, not build time, if OpenCV isn't present — so building just the
# engine/networking/server/tests needs this off, not just left unbuilt.
option(KFC_BUILD_UI "Build the OpenCV-based windowed UI and its Kung-Fu-Chess executable (requires vendored OpenCV)" ON)

if(KFC_BUILD_UI)
    add_subdirectory(Kung-Fu-Chess.UI)
endif()
add_subdirectory(Kung-Fu-Chess.Net)
add_subdirectory(Kung-Fu-Chess.Server)
add_subdirectory(Kung-Fu-Chess.Client)
if(KFC_BUILD_UI)
    add_subdirectory(Kung-Fu-Chess)
endif()
add_subdirectory(Kung-Fu-Chess.Tests)
