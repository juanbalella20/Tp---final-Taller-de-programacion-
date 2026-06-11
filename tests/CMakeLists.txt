# ── GoogleTest via FetchContent ──────────────────────────────────────────────
# Se baja en tiempo de configuracion para no depender de libgtest-dev del sistema.
include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        v1.14.0
)
# En Windows evita sobrescribir la config de runtime del padre (inofensivo en Linux).
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

include(GoogleTest)

# ── Fuentes del protocolo bajo test ──────────────────────────────────────────
# Los (de)serializers viven en client/ y server/, no en la lib `common`. Son C++
# puro (solo <arpa/inet.h>), asi que se compilan directo en el binario de tests.
# `common` aporta ClientCmd y GameMsg.
add_executable(protocol_tests
    test_client_protocol.cpp
    test_server_protocol.cpp
    ${CMAKE_SOURCE_DIR}/client/serializers/serializer.cpp
    ${CMAKE_SOURCE_DIR}/client/serializers/deserializer.cpp
    ${CMAKE_SOURCE_DIR}/server/serializer/serverSerializer.cpp
    ${CMAKE_SOURCE_DIR}/server/serializer/serverDeserializer.cpp
)

target_include_directories(protocol_tests PRIVATE
    ${CMAKE_SOURCE_DIR}/common/commands
    ${CMAKE_SOURCE_DIR}/common/constants
    ${CMAKE_SOURCE_DIR}/common/info
    ${CMAKE_SOURCE_DIR}/client/serializers
    ${CMAKE_SOURCE_DIR}/server/serializer
)

target_link_libraries(protocol_tests PRIVATE
    common
    GTest::gtest_main
)

gtest_discover_tests(protocol_tests)
