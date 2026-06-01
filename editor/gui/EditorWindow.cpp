#include "EditorWindow.h"

#include <QAction>
#include <QDockWidget>
#include <QKeySequence>
#include <QMenu>
#include <QMenuBar>
#include <QScrollArea>
#include <QStatusBar>

#include "../render/MapCanvasWidget.h"
#include "Map.h"
#include "TilePalette.h"

EditorWindow::EditorWindow(QWidget* parent) : QMainWindow(parent) {
    setWindowTitle("Editor de Mapas - Argentum Online");
    resize(1024, 768);

    build_menus();
    build_canvas();
    build_palette_dock();

    statusBar()->showMessage("Listo");
}

void EditorWindow::build_canvas() {
    canvas_ = new MapCanvasWidget(&map_, this);
    auto* scroll = new QScrollArea(this);
    scroll->setWidget(canvas_);
    scroll->setAlignment(Qt::AlignCenter);
    setCentralWidget(scroll);
}

void EditorWindow::build_palette_dock() {
    palette_ = new TilePalette(this);
    auto* dock = new QDockWidget("Tiles", this);
    dock->setWidget(palette_);
    addDockWidget(Qt::LeftDockWidgetArea, dock);

    // Mas adelante esto ira al EditorDocument
    connect(palette_, &TilePalette::tileSelected, this, [this](int gid) {
        statusBar()->showMessage(QString("Tile seleccionado: gid %1").arg(gid));
    });

    // Al cargar un PNG, registrar el tileset en el Map y actualizar canvas.
    connect(palette_, &TilePalette::tilesetLoaded, this,
            [this](const QString& name, const QString& file_path, int columns,
                   int tile_count, bool collidable) {
                map_.add_tileset(name.toStdString(), file_path.toStdString(),
                                 columns, tile_count, collidable);
                if (canvas_) canvas_->refresh();
            });
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
void EditorWindow::on_save() { statusBar()->showMessage("Guardar mapa (pendiente)"); }
void EditorWindow::on_save_as() { statusBar()->showMessage("Guardar como (pendiente)"); }
