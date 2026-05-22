cmake_minimum_required(VERSION 3.16)
project(argentum CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")

# ── Vendored submodules (SDL3, SDL3_image) ──────────────────────────────────
# If submodules aren't initialized, try to do it automatically; otherwise fail
# with a clear message instead of a cryptic "CMakeLists.txt not found".
function(ensure_submodule path)
    if(EXISTS "${CMAKE_SOURCE_DIR}/${path}/CMakeLists.txt")
        return()
    endif()

    find_package(Git QUIET)
    if(GIT_FOUND AND EXISTS "${CMAKE_SOURCE_DIR}/.git")
        message(STATUS "Submodule '${path}' not initialized — running git submodule update --init --recursive")
        execute_process(
            COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive -- ${path}
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            RESULT_VARIABLE _git_result
        )
        if(NOT _git_result EQUAL 0)
            message(FATAL_ERROR
                "Failed to initialize submodule '${path}' (git exited with ${_git_result}).\n"
                "Run manually: git submodule update --init --recursive")
        endif()
    endif()

    if(NOT EXISTS "${CMAKE_SOURCE_DIR}/${path}/CMakeLists.txt")
        message(FATAL_ERROR
            "Submodule '${path}' is missing.\n"
            "This project depends on vendored SDL3 sources. Initialize submodules with:\n"
            "    git submodule update --init --recursive\n"
            "Or clone with: git clone --recurse-submodules <url>")
    endif()
endfunction()

ensure_submodule(vendored/SDL)
ensure_submodule(vendored/SDL_image)

# SDL3 from vendored source
add_subdirectory(vendored/SDL EXCLUDE_FROM_ALL)
add_subdirectory(vendored/SDL_image EXCLUDE_FROM_ALL)
add_subdirectory(vendored/SDL_ttf EXCLUDE_FROM_ALL)

# ── Common library (shared by client and server) ────────────────────────────
add_library(common STATIC
    common/socket.cpp
    common/resolver.cpp
    common/resolvererror.cpp
    common/liberror.cpp
    common/clientCmd.cpp
    common/gameMsg.cpp
    common/player.cpp
    common/player_class.cpp 
    common/player_race.cpp
    # TODO: add when Item type exists:
    # common/inventory.cpp common/player_class.cpp common/player_race.cpp
)
target_include_directories(common PUBLIC common)

# ── Client ──────────────────────────────────────────────────────────────────
add_executable(client
    client/clientmain.cpp
    client/clientApp.cpp
    client/minichat.cpp
    client/clientProtocol.cpp
    client/deserializer.cpp
    client/serialze.cpp
    client/parser.cpp
    client/clientGUI.cpp
    client/tileMap.cpp
    client/displayPlayer.cpp
    client/networkSenderThread.cpp
    client/networkReceiverThread.cpp
)
target_include_directories(client PRIVATE client)
target_link_libraries(client PRIVATE common SDL3::SDL3 SDL3_image::SDL3_image)
# find_package(SDL3_ttf REQUIRED)
target_link_libraries(client PRIVATE SDL3_ttf::SDL3_ttf)

# ── GUI test (SDL3 sandbox) ─────────────────────────────────────────────────
# add_executable(gui_test
#     client/guiTest.cpp
#     client/clientGUI.cpp
#     client/displayPlayer.cpp
# )
# target_link_libraries(gui_test PRIVATE common SDL3::SDL3 SDL3_image::SDL3_image)

# ── Server ──────────────────────────────────────────────────────────────────
add_executable(server
    server/server.cpp
    server/serverApp.cpp
    server/acceptor.cpp
    server/clientHandler.cpp
    server/clientRegistryMonitor.cpp
    server/receiverThread.cpp
    server/senderThread.cpp
    server/serverProtocol.cpp
    server/serverDeserializer.cpp
    server/serverSerializer.cpp
    server/gameloop.cpp
    server/game_map.cpp
    # not implemented yet
    # server/npcBanker.cpp
    # server/npcHostile.cpp
    # server/npcPriest.cpp
    # server/npcSeller.cpp
)
target_include_directories(server PRIVATE server)
target_link_libraries(server PRIVATE common)

