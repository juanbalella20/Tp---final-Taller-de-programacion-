# ── Editor grafico de mapas (Qt6) ───────────────────────────────────────────
find_package(Qt6 REQUIRED COMPONENTS Widgets)

# moc/uic/rcc automaticos para clases con Q_OBJECT
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

add_executable(map_editor
    main.cpp
    model/Map.cpp
    gui/EditorWindow.cpp
    gui/TilePalette.cpp
    gui/TilePalette.ui
    render/MapCanvasWidget.cpp
    render/TilesetPixmapCache.cpp
    document/EditorDocument.cpp
    document/SetTilesCommand.cpp
    document/ToggleTeleportCommand.cpp
    tools/PencilTool.cpp
    tools/EraserTool.cpp
    tools/FillTool.cpp
)

target_include_directories(map_editor PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/model
    ${CMAKE_CURRENT_SOURCE_DIR}/document
    ${CMAKE_CURRENT_SOURCE_DIR}/tools
    ${CMAKE_CURRENT_SOURCE_DIR}/render
    ${CMAKE_CURRENT_SOURCE_DIR}/gui
)

target_link_libraries(map_editor PRIVATE common Qt6::Widgets)

set_target_properties(map_editor PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
)
