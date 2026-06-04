#ifndef EDITOR_WINDOW_H
#define EDITOR_WINDOW_H

#include <QMainWindow>
#include <QString>

#include "../document/EditorDocument.h"

class TilePalette;
class MapCanvasWidget;
class QAction;

class EditorWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit EditorWindow(QWidget* parent = nullptr);

private slots:
    void on_new();
    void on_open();
    void on_save();
    void on_save_as();

private:
    // Crea menu: archivo,nuevo,abrir,guardar, guardar como, salir
    void build_menus();
    // Crea widget de tiles
    void build_palette_dock();
    // Crea el grid del mapa
    void build_canvas();
    // Crea la toolbar: herramientas (lapiz/goma/relleno) y capa activa.
    void build_tools_toolbar();
    // Crea menu Editar (deshacer/rehacer) y guarda las acciones.
    void build_edit_menu();
    // Habilita/deshabilita deshacer/rehacer segun el estado del document.
    void update_undo_actions();

    // Serializa el Map actual a 'path' (.bin via BinaryMapSaver). Muestra el
    // resultado en la status bar; devuelve true si se guardo bien.
    bool save_to(const QString& path);

    // Controlador: dueno del Map, del estado de edicion y del undo/redo.
    EditorDocument doc_;
    TilePalette* palette_ = nullptr;
    MapCanvasWidget* canvas_ = nullptr;
    QAction* undo_action_ = nullptr;
    QAction* redo_action_ = nullptr;
    // Ruta del archivo actual (vacia si nunca se guardo). on_save reusa esta
    // ruta; si esta vacia delega en on_save_as.
    QString current_path_;
};

#endif
