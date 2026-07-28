enable_language(C) # for the vendored sha256.c / sqlite3.c amalgamation

set(KFC_ENGINE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../Kung-Fu-Chess)

add_library(Kung-Fu-Chess.Server STATIC
    src/EventBus.cpp
    src/RatingManager.cpp
    src/GameRoom.cpp
    src/GameManager.cpp
    src/Matchmaker.cpp
    src/PasswordHasher.cpp
    src/InMemoryUserRepository.cpp
    src/SqliteUserRepository.cpp
    src/ConnectionManager.cpp
    src/ThirdParty/sha256.c
    src/ThirdParty/sqlite3.c
)

# The vendored SQLite amalgamation is plain C with a couple of warnings
# under MSVC's C++ warning set; it isn't our code to clean up.
set_source_files_properties(src/ThirdParty/sqlite3.c src/ThirdParty/sha256.c PROPERTIES COMPILE_OPTIONS "/W0")

target_include_directories(Kung-Fu-Chess.Server PUBLIC
    include
    ${KFC_ENGINE_DIR}/include
)
target_include_directories(Kung-Fu-Chess.Server PRIVATE
    include/ThirdParty
    include/ThirdParty/sqlite
)

target_link_libraries(Kung-Fu-Chess.Server PUBLIC Kung-Fu-Chess.Net)
