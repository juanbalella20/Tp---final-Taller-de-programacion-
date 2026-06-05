#include "EditorWindow.h"

#include <QAction>
#include <QActionGroup>
#include <QComboBox>
#include <QDockWidget>
#include <QFileDialog>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QKeySequence>
#include <QLabel>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QStatusBar>
#include <QToolBar>
#include <QWidget>

#include <exception>

#include "../model/Map.h"
#include "../render/MapCanvasWidget.h"
#include "TilePalette.h"
#include "binaryMap/binaryMapSaver.h"
#include "protocol_constants.h"  // Zone, ZONE_NAME_MAP_INV

EditorWindow::EditorWindow(QWidget* parent) : QMainWindow(parent) {
    setWindowTitle("Editor de Mapas - Argentum Online");
    resize(1024, 768);

    build_menus();
    build_edit_menu();
    build_canvas();
    build_palette_dock();
    build_tools_toolbar();

    update_undo_actions();
    statusBar()->showMessage("Listo");
}

void EditorWindow::build_canvas() {
    // El canvas se escala solo para que el mapa entre siempre en pantalla, sin
    // scroll: va directo como widget central.
    canvas_ = new MapCanvasWidget(&doc_, this);
    setCentralWidget(canvas_);
}

void EditorWindow::build_palette_dock() {
    palette_ = new TilePalette(this);
    auto* dock = new QDockWidget("Tiles", this);
    dock->setWidget(palette_);
    addDockWidget(Qt::LeftDockWidgetArea, dock);

    // El tile seleccionado en la paleta pasa a ser el brush activo del document.
    connect(palette_, &TilePalette::tileSelected, this, [this](int gid) {
        doc_.set_active_gid(gid);
        statusBar()->showMessage(QString("Tile seleccionado: gid %1").arg(gid));
    });

    // Al cargar un PNG, registrar el tileset en el Map (via document) y refrescar.
    connect(palette_, &TilePalette::tilesetLoaded, this,
            [this](const QString& name, const QString& file_path, int columns,
                   int tile_count, bool collidable) {
                doc_.map().add_tileset(name.toStdString(),
                                       file_path.toStdString(), columns,
                                       tile_count, collidable);
                if (canvas_) canvas_->refresh();
            });
}

void EditorWindow::build_tools_toolbar() {
    auto* toolbar = addToolBar("Herramientas");

    // Todas las herramientas (pintura + Teleport) son mutuamente exclusivas.
    auto* tools = new QActionGroup(this);
    tools->setExclusive(true);

    auto add_tool = [&](const QString& text, ToolType type, bool checked) {
        QAction* act = toolbar->addAction(text);
        act->setCheckable(true);
        act->setChecked(checked);
        tools->addAction(act);
        connect(act, &QAction::triggered, this,
                [this, type] { doc_.set_active_tool(type); });
        return act;
    };

    // --- Grupo 1: herramientas de pintura --------------------------------
    add_tool("Lapiz", ToolType::Pencil, true);  // default
    add_tool("Goma", ToolType::Eraser, false);
    add_tool("Relleno", ToolType::Fill, false);

    toolbar->addSeparator();

    // --- Grupo 2: Teleport + capas de tiles ------------------------------
    QAction* teleport_tool = add_tool("Teleport", ToolType::Teleport, false);

    // Combo de zona destino (label + combo en un solo widget). Define la zona a
    // la que apuntan las celdas marcadas con Teleport. Solo visible con esa
    // herramienta activa.
    auto* dest_widget = new QWidget(this);
    auto* dest_layout = new QHBoxLayout(dest_widget);
    dest_layout->setContentsMargins(4, 0, 4, 0);
    dest_layout->addWidget(new QLabel("Destino: "));
    auto* dest_combo = new QComboBox(dest_widget);
    for (Zone z : {ZONE_DESERT, ZONE_CITY, ZONE_FOREST, ZONE_TOWN}) {
        dest_combo->addItem(QString::fromStdString(ZONE_NAME_MAP_INV.at(z)));
    }
    dest_combo->setCurrentText(QString::fromStdString(doc_.active_dest_zone()));
    dest_layout->addWidget(dest_combo);
    dest_combo_action_ = toolbar->addWidget(dest_widget);
    dest_combo_action_->setVisible(false);  // oculto hasta activar Teleport

    connect(dest_combo, &QComboBox::currentTextChanged, this,
            [this](const QString& zone) {
                doc_.set_active_dest_zone(zone.toStdString());
            });
    // Mostrar el combo solo cuando Teleport esta activa. setChecked(false) en una
    // accion del grupo exclusivo tambien dispara toggled, asi cubrimos ambos lados.
    connect(teleport_tool, &QAction::toggled, this, [this](bool on) {
        if (dest_combo_action_) dest_combo_action_->setVisible(on);
    });

    // Capas de tiles editables (exclusivas entre si).
    auto* layers = new QActionGroup(this);
    layers->setExclusive(true);

    auto add_layer = [&](const QString& text, int idx, bool checked) {
        QAction* act = toolbar->addAction(text);
        act->setCheckable(true);
        act->setChecked(checked);
        layers->addAction(act);
        connect(act, &QAction::triggered, this,
                [this, idx] { doc_.set_active_layer(idx); });
    };
    add_layer("Suelo", Map::Ground, true);  // default
    add_layer("Construcciones", Map::Buildings, false);
}

void EditorWindow::build_edit_menu() {
    QMenu* editar = menuBar()->addMenu("&Editar");

    undo_action_ = editar->addAction("&Deshacer");
    undo_action_->setShortcut(QKeySequence::Undo);  // Ctrl+Z
    connect(undo_action_, &QAction::triggered, this, [this] { doc_.undo(); });

    redo_action_ = editar->addAction("&Rehacer");
    redo_action_->setShortcut(QKeySequence::Redo);  // Ctrl+Shift+Z / Ctrl+Y
    connect(redo_action_, &QAction::triggered, this, [this] { doc_.redo(); });

    // El document avisa cuando cambian las pilas -> refrescamos el enable.
    connect(&doc_, &EditorDocument::undoStackChanged, this,
            &EditorWindow::update_undo_actions);
}

void EditorWindow::update_undo_actions() {
    if (undo_action_) undo_action_->setEnabled(doc_.can_undo());
    if (redo_action_) redo_action_->setEnabled(doc_.can_redo());
}

void EditorWindow::build_menus() {
    QMenu* archivo = menuBar()->addMenu("&Archivo");

    QAction* nuevo = archivo->addAction("&Nuevo");
    nuevo->setShortcut(QKeySequence::New);
    connect(nuevo, &QAction::triggered, this, &EditorWindow::on_new);

    QAction* abrir = archivo->addAction("&Abrir...");
    abrir->setShortcut(QKeySequence::Open);
    connect(abrir, &QAction::triggered, this, &EditorWindow::on_open);

    archivo->addSeparator();

    QAction* guardar = archivo->addAction("&Guardar");
    guardar->setShortcut(QKeySequence::Save);
    connect(guardar, &QAction::triggered, this, &EditorWindow::on_save);

    QAction* guardar_como = archivo->addAction("Guardar &como...");
    guardar_como->setShortcut(QKeySequence::SaveAs);
    connect(guardar_como, &QAction::triggered, this, &EditorWindow::on_save_as);

    archivo->addSeparator();

    QAction* salir = archivo->addAction("&Salir");
    salir->setShortcut(QKeySequence::Quit);
    connect(salir, &QAction::triggered, this, &QWidget::close);
}

void EditorWindow::on_new() { statusBar()->showMessage("Nuevo mapa (pendiente)"); }
void EditorWindow::on_open() { statusBar()->showMessage("Abrir mapa (pendiente)"); }
void EditorWindow::on_save() {
    // Sin ruta previa (mapa nunca guardado): comportarse como "Guardar como".
    if (current_path_.isEmpty()) {
        on_save_as();
        return;
    }
    save_to(current_path_);
}

void EditorWindow::on_save_as() {
    QString path = QFileDialog::getSaveFileName(
        this, "Guardar mapa", current_path_, "Mapa binario (*.bin)");
    if (path.isEmpty()) return;  // cancelado

    // Asegura la extension .bin si el usuario no la escribio.
    if (QFileInfo(path).suffix().isEmpty()) path += ".bin";

    if (save_to(path)) current_path_ = path;
}

bool EditorWindow::save_to(const QString& path) {
    const Map& map = doc_.map();
    try {
        BinaryMapSaver::save(path.toStdString(), map.tile_size(), map.width(),
                             map.height(), map.tilesets(), map.layers(),
                             map.teleports());
    } catch (const std::exception& e) {
        QMessageBox::warning(this, "Error al guardar",
                             QString("No se pudo guardar el mapa:\n%1").arg(e.what()));
        statusBar()->showMessage("Error al guardar");
        return false;
    }

    statusBar()->showMessage(QString("Mapa guardado: %1").arg(path));
    return true;
}
