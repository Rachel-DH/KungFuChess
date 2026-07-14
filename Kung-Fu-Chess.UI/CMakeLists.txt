find_package(OpenCV CONFIG REQUIRED)

add_executable(Kung-Fu-Chess.UI
    src/main.cpp
)

target_include_directories(Kung-Fu-Chess.UI PRIVATE include)
target_link_libraries(Kung-Fu-Chess.UI PRIVATE ${OpenCV_LIBS})
