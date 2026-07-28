set(KFC_ENGINE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../Kung-Fu-Chess)

add_library(Kung-Fu-Chess.Client STATIC
    src/ClientState.cpp
    src/ViewModel.cpp
)

target_include_directories(Kung-Fu-Chess.Client PUBLIC
    include
    ${KFC_ENGINE_DIR}/include
)

target_link_libraries(Kung-Fu-Chess.Client PUBLIC Kung-Fu-Chess.Net)
