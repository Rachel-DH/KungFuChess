set(KFC_ENGINE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../Kung-Fu-Chess)

add_library(Kung-Fu-Chess.Server STATIC
    src/EventBus.cpp
    src/RatingManager.cpp
    src/GameRoom.cpp
    src/GameManager.cpp
)

target_include_directories(Kung-Fu-Chess.Server PUBLIC
    include
    ${KFC_ENGINE_DIR}/include
)

target_link_libraries(Kung-Fu-Chess.Server PUBLIC Kung-Fu-Chess.Net)
