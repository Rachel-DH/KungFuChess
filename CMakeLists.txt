cmake_minimum_required(VERSION 3.20)

option(KFC_BUILD_UI "Build Kung-Fu-Chess.UI (requires OpenCV via vcpkg)" ON)

# Pick up the vendored vcpkg toolchain automatically, but only when the UI
# subproject (the only target needing OpenCV) is actually being built.
if(KFC_BUILD_UI AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(_kfc_vcpkg_toolchain "${CMAKE_CURRENT_SOURCE_DIR}/tools/vcpkg/scripts/buildsystems/vcpkg.cmake")
    if(EXISTS "${_kfc_vcpkg_toolchain}")
        set(CMAKE_TOOLCHAIN_FILE "${_kfc_vcpkg_toolchain}" CACHE FILEPATH "")
    endif()
    unset(_kfc_vcpkg_toolchain)
endif()

project(KungFuChess CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(Kung-Fu-Chess)
add_subdirectory(Kung-Fu-Chess.Tests)

if(KFC_BUILD_UI)
    add_subdirectory(Kung-Fu-Chess.UI)
endif()
