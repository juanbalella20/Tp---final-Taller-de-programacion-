# Tests del nucleo del editor (Persona A): formato binario de mapas.
# Sin Qt; solo dependen del target 'common' y de GoogleTest.

include(GoogleTest)

add_executable(test_binary_map
    test_binary_map.cpp
)
# C++17 alcanza para estos tests (structured bindings, std::filesystem).
target_link_libraries(test_binary_map PRIVATE common GTest::gtest GTest::gtest_main)

# Los tests que leen data/maps/... usan paths relativos: correr desde la raiz.
gtest_discover_tests(test_binary_map
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)
