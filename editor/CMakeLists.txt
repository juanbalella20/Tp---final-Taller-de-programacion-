# Editor grafico de mapas (Qt6)
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(MAP_EDITOR_CORE_SOURCES
    model/Map.cpp
    gui/EditorWindow.cpp
    gui/TilesetSelectorView.cpp
    render/MapEditorScene.cpp
    render/TileLibrary.cpp
    document/EditorDocument.cpp
    tools/PencilTool.cpp
    tools/EraserTool.cpp
    tools/FillTool.cpp
)

add_library(map_editor_core STATIC ${MAP_EDITOR_CORE_SOURCES})
target_include_directories(map_editor_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/model
    ${CMAKE_CURRENT_SOURCE_DIR}/document
    ${CMAKE_CURRENT_SOURCE_DIR}/tools
    ${CMAKE_CURRENT_SOURCE_DIR}/render
    ${CMAKE_CURRENT_SOURCE_DIR}/gui
)
target_link_libraries(map_editor_core PUBLIC
    common
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
)
target_compile_features(map_editor_core PUBLIC cxx_std_20)
#target_compile_definitions(map_editor_core PRIVATE
#    EDITOR_DEFAULT_TILESET_PATH="${CMAKE_SOURCE_DIR}/data/tiles/spritesheet.png"
#)

add_executable(map_editor main.cpp)
target_link_libraries(map_editor PRIVATE map_editor_core)
target_compile_features(map_editor PRIVATE cxx_std_20)

# Instalacion: el editor va junto a client/server en <prefix>/bin (~/.local/bin).
# Qt6 es del sistema (apt), asi que no hay libs propias que copiar.
install(TARGETS map_editor RUNTIME DESTINATION bin)
