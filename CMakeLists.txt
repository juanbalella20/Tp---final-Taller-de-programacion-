cmake_minimum_required(VERSION 3.16)
project(argentum CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g3 -ggdb3 -fno-omit-frame-pointer -fno-inline -fno-eliminate-unused-debug-types")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")

# ── Vendored submodules (SDL3, SDL3_image, SDL3_ttf) ────────────────────────
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
ensure_submodule(vendored/SDL_ttf)

# Build SDL_ttf with vendored third-party deps (freetype/harfbuzz/plutosvg)
# so external users don't need to apt install extra dev packages.
set(SDLTTF_VENDORED ON CACHE BOOL "Build SDL_ttf with vendored dependencies" FORCE)

# SDL3 from vendored source
add_subdirectory(vendored/SDL EXCLUDE_FROM_ALL)
add_subdirectory(vendored/SDL_image EXCLUDE_FROM_ALL)
add_subdirectory(vendored/SDL_ttf EXCLUDE_FROM_ALL)

# ── Common library (shared by client and server) ────────────────────────────
add_library(common STATIC
    common/socket/socket.cpp
    common/socket/resolver.cpp
    common/socket/resolvererror.cpp
    common/socket/liberror.cpp
    common/commands/clientCmd.cpp
    common/commands/gameMsg.cpp
    common/mapLoader.cpp
    common/binaryMap/binaryMapLoader.cpp
    common/binaryMap/binaryMapSaver.cpp
    common/binaryMap/byteReader.cpp
    common/utility/paths.cpp
    common/constants/game_config.cpp
    common/constants/network_config.cpp
)
target_include_directories(common PUBLIC
    common
    common/socket
    common/commands
    common/constants
    common/info
    common/utility
    common/binaryMap
    vendored/tomlplusplus
)

# ── Client ──────────────────────────────────────────────────────────────────
add_executable(client
    client/clientmain.cpp
    client/clientApp.cpp
    client/interface/minichat.cpp
    client/comunication/clientProtocol.cpp
    client/serializers/deserializer.cpp
    client/serializers/serializer.cpp
    client/parser.cpp
    client/interface/clientGUI.cpp
    client/interface/welcome_screen.cpp
    client/interface/screen_manager.cpp
    client/interface/login_signup_screen.cpp
    client/interface/character_creation_screen.cpp
    client/interface/tileMap.cpp
    client/interface/displayPlayer.cpp
    client/interface/camera.cpp
    client/comunication/networkSenderThread.cpp
    client/comunication/networkReceiverThread.cpp
    client/interface/npcSprite.cpp
    client/interface/itemSprite.cpp
    client/interface/hud.cpp
    client/interface/ui_helpers.cpp
    client/interface/text_field.cpp
)
target_include_directories(client PRIVATE
    client
    client/interface
    client/comunication
    client/serializers
)
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
    server/communication/acceptor.cpp
    server/communication/clientHandler.cpp
    server/communication/clientRegistryMonitor.cpp
    server/communication/receiverThread.cpp
    server/communication/senderThread.cpp
    server/communication/serverProtocol.cpp
    server/serializer/serverDeserializer.cpp
    server/serializer/serverSerializer.cpp
    server/gameloop.cpp
    server/Game/game_map.cpp
    server/Game/zoneWorld.cpp
    server/Game/player/player.cpp
    server/Game/player/player_class.cpp
    server/Game/player/player_race.cpp
    server/Game/player/level.cpp
    server/Game/player/alive_state.cpp
    server/Game/player/ghost_state.cpp
    server/Game/player/meditate_state.cpp
    server/Game/player/clan.cpp
    server/Game/inventory.cpp
    server/Game/defense_set.cpp
    server/Game/item/item.cpp
    server/Game/item/arma.cpp
    server/Game/item/baculo.cpp
    server/Game/item/defense_item.cpp
    server/Game/item/armadura.cpp
    server/Game/item/casco.cpp
    server/Game/item/escudo.cpp
    server/Game/item/item_catalog.cpp
    server/persistence/player_persistence.cpp
    server/persistence/player_serializer.cpp
    server/persistence/auth_service.cpp
    server/Game/npc/npc.cpp
    server/Game/npc/npcHostile.cpp
    # server/Game/npc/npcBanker.cpp
    # server/Game/npc/npcPriest.cpp
    server/Game/npc/npcSeller.cpp
)
target_include_directories(server PRIVATE
    server
    server/communication
    server/serializer
    server/Game
    server/Game/item
    server/Game/npc
    server/Game/player
    server/persistence
)
target_link_libraries(server PRIVATE common)

# ── Editor grafico de mapas (Qt6) ────────────────────────────────────────────
# Se construye solo si Qt6 Widgets esta disponible, para no romper el build de
# client/server en entornos sin qt6-base-dev. Instalar con: apt install qt6-base-dev
find_package(Qt6 QUIET COMPONENTS Widgets)
if(Qt6_FOUND)
    add_subdirectory(editor)
else()
    message(WARNING "Qt6 Widgets no encontrado: se omite el target map_editor. "
                    "Instalar con: sudo apt install qt6-base-dev")
endif()
