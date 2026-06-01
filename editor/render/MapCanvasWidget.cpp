#include "MapCanvasWidget.h"

#include <QPainter>

#include "Map.h"
#include "game_constants.h"  // WIDTH, HEIGHT, TILE_SIZE

MapCanvasWidget::MapCanvasWidget(Map* map, QWidget* parent)
    : QWidget(parent), map(map) {
    // El mapa es de tamaño fijo: WIDTH x HEIGHT celdas de TILE_SIZE px.
    setFixedSize(WIDTH * TILE_SIZE, HEIGHT * TILE_SIZE);
}

void MapCanvasWidget::refresh() {
    if (map) cache.sync(*map);
    update();
}

void MapCanvasWidget::paintEvent(QPaintEvent* /*event*/) {
    QPainter painter(this);

    // Fondo de las celdas vacias.
    painter.fillRect(rect(), QColor(0, 0, 0));

    // Tiles del mapa, capa por capa (algoritmo del pintor)
    if (map) {
        for (int layer = 0; layer < map->layer_count(); ++layer) {
            for (int y = 0; y < HEIGHT; ++y) {
                for (int x = 0; x < WIDTH; ++x) {
                    int gid = map->get_cell(layer, x, y);
                    if (gid == 0) continue;  // celda vacia

                    const TileDef* td = map->find_tile(gid);
                    if (!td) continue;
                    const QPixmap* sheet = cache.pixmap_for_tileset(td->tileset_index);
                    QRect dst(x * TILE_SIZE, y * TILE_SIZE, TILE_SIZE, TILE_SIZE);

                    if (sheet && !sheet->isNull()) {
                        painter.drawPixmap(dst, *sheet, cache.src_rect(*map, gid));
                    } else {
                        // Fallback de color si el PNG no cargo: color por gid.
                        painter.fillRect(dst, QColor::fromHsv((gid * 47) % 360, 160, 200));
                    }
                }
            }
        }
    }

    // Grilla: lineas verticales y horizontales cada TILE_SIZE px.
    painter.setPen(QColor(70, 70, 70));
    for (int col = 0; col <= WIDTH; ++col) {
        int x = col * TILE_SIZE;
        painter.drawLine(x, 0, x, HEIGHT * TILE_SIZE);
    }
    for (int row = 0; row <= HEIGHT; ++row) {
        int y = row * TILE_SIZE;
        painter.drawLine(0, y, WIDTH * TILE_SIZE, y);
    }
}


