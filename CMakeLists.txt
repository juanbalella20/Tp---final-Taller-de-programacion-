cmake_minimum_required(VERSION 3.16)
project(argentum CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")

# SDL3 from vendored source (required)

add_subdirectory(vendored/SDL EXCLUDE_FROM_ALL)
add_subdirectory(vendored/SDL_image EXCLUDE_FROM_ALL)

# ── Common library (shared by client and server) ────────────────────────────
add_library(common STATIC
    common/socket.cpp
    common/resolver.cpp
    common/resolvererror.cpp
    common/liberror.cpp
    common/clientCmd.cpp
    common/gameMsg.cpp
)
target_include_directories(common PUBLIC common)

# ── Client ──────────────────────────────────────────────────────────────────
add_executable(client
    client/clientmain.cpp
    client/clientApp.cpp
    client/clientProtocol.cpp
    client/deserializer.cpp
    client/serialze.cpp
    client/parser.cpp
    client/clientGUI.cpp
    client/displayPlayer.cpp
    client/networkSenderThread.cpp
    client/networkReceiverThread.cpp

)
target_include_directories(client PRIVATE client)
target_link_libraries(client PRIVATE common SDL3::SDL3 SDL3_image::SDL3_image)

# ── GUI test (SDL3 sandbox) ─────────────────────────────────────────────────
add_executable(gui_test
    client/guiTest.cpp
    client/clientGUI.cpp
    client/displayPlayer.cpp
)
target_link_libraries(gui_test PRIVATE common SDL3::SDL3 SDL3_image::SDL3_image)

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
)
target_include_directories(server PRIVATE server)
target_link_libraries(server PRIVATE common)
