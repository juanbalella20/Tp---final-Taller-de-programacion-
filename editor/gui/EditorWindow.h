#ifndef EDITOR_WINDOW_H
#define EDITOR_WINDOW_H

#include <QMainWindow>

#include "Map.h"

class TilePalette;
class MapCanvasWidget;

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

    // Modelo del mapa (mas adelante pasa a EditorDocument).
    Map map_;
    TilePalette* palette_ = nullptr;
    MapCanvasWidget* canvas_ = nullptr;
};

#endif
