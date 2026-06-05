#ifndef MAP_CANVAS_WIDGET_H
#define MAP_CANVAS_WIDGET_H

#include <QWidget>

#include "TilesetPixmapCache.h"

class EditorDocument;


class MapCanvasWidget : public QWidget {
    Q_OBJECT

public:
    explicit MapCanvasWidget(EditorDocument* doc, QWidget* parent = nullptr);

public slots:
    // Resincroniza el cache de pixmaps con los tilesets del Map y repinta.
    // Llamar tras cargar un tileset o tras abrir/crear un mapa.
    void refresh();

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;

private:
    EditorDocument* doc_;
    TilesetPixmapCache cache_;

    // Tamaño en pixeles de cada tile, calculado para que el mapa completo entre
    // en el area disponible del widget manteniendo la relacion de aspecto.
    int tile_px() const;
    // Origen (esquina sup-izq) del mapa dentro del widget, para centrarlo.
    QPoint origin() const;

    // Convierte coordenadas de pixel del widget a celda (col,row). Devuelve
    // false si el punto cae fuera del grid.
    bool cell_at(const QPoint& pos, int* cx, int* cy) const;
};

#endif  // MAP_CANVAS_WIDGET_H
