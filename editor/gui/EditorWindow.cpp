#include "EditorWindow.h"

#include <QAction>
#include <QKeySequence>
#include <QLabel>
#include <QMenu>
#include <QMenuBar>
#include <QStatusBar>

EditorWindow::EditorWindow(QWidget* parent) : QMainWindow(parent) {
    setWindowTitle("Editor de Mapas - Argentum Online");
    resize(1024, 768);

    build_menus();

    auto* placeholder = new QLabel("Area de edicion del mapa", this);
    placeholder->setAlignment(Qt::AlignCenter);
    setCentralWidget(placeholder);

    statusBar()->showMessage("Listo");
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
