#ifndef MAP_CANVAS_WIDGET_H
#define MAP_CANVAS_WIDGET_H

#include <QWidget>

#include "TilesetPixmapCache.h"
#include "Map.h"


class MapCanvasWidget : public QWidget {
    Q_OBJECT

public:
    explicit MapCanvasWidget(Map* map, QWidget* parent = nullptr);

public slots:
    // Resincroniza el cache de pixmaps con los tilesets del Map y repinta.
    // Llamar tras cargar un tileset o tras abrir/crear un mapa.
    void refresh();

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    Map* map;
    TilesetPixmapCache cache;
};

#endif  // MAP_CANVAS_WIDGET_H
