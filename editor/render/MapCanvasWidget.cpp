#include "MapCanvasWidget.h"

#include <QMouseEvent>
#include <QPainter>
#include <algorithm>  // std::min

#include "../document/EditorDocument.h"
#include "Map.h"
#include "game_constants.h"  // WIDTH, HEIGHT, TILE_SIZE

MapCanvasWidget::MapCanvasWidget(EditorDocument* doc, QWidget* parent)
    : QWidget(parent), doc_(doc) {
    // El canvas se adapta al area disponible (sin tamaño fijo): el mapa completo
    // siempre entra en pantalla escalando el tamaño del tile y manteniendo la
    // relacion de aspecto WIDTH:HEIGHT, centrado en el widget.
    setMinimumSize(WIDTH, HEIGHT);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    if (doc_) {
        // Una celda cambio: por ahora repintamos todo (mapa chico, 30x16). Mas
        // adelante se puede invalidar solo el rect de esa celda.
        connect(doc_, &EditorDocument::cellChanged, this,
                [this](int, int, int) { update(); });
        // Tras new/open: resincronizar tilesets y repintar todo.
        connect(doc_, &EditorDocument::mapReset, this,
                &MapCanvasWidget::refresh);
    }
}

void MapCanvasWidget::refresh() {
    if (doc_) cache_.sync(doc_->map());
    update();
}

int MapCanvasWidget::tile_px() const {
    // El tile mas grande que permite encajar WIDTH x HEIGHT celdas en el widget.
    int by_w = width() / WIDTH;
    int by_h = height() / HEIGHT;
    return std::max(1, std::min(by_w, by_h));
}

QPoint MapCanvasWidget::origin() const {
    // Centra el mapa dentro del widget (los margenes sobran por el aspect ratio).
    int ts = tile_px();
    int mx = (width() - WIDTH * ts) / 2;
    int my = (height() - HEIGHT * ts) / 2;
    return QPoint(mx, my);
}

bool MapCanvasWidget::cell_at(const QPoint& pos, int* cx, int* cy) const {
    int ts = tile_px();
    QPoint o = origin();
    int x = (pos.x() - o.x()) / ts;
    int y = (pos.y() - o.y()) / ts;
    // Descarta clicks en el margen (pos < origen daria valores negativos que la
    // division truncaria hacia 0 en vez de fuera de rango).
    if (pos.x() < o.x() || pos.y() < o.y()) return false;
    if (x < 0 || y < 0 || x >= WIDTH || y >= HEIGHT) return false;
    *cx = x;
    *cy = y;
    return true;
}

void MapCanvasWidget::mousePressEvent(QMouseEvent* event) {
    int cx, cy;
    if (doc_ && event->button() == Qt::LeftButton &&
        cell_at(event->pos(), &cx, &cy)) {
        doc_->apply_tool_press(cx, cy);
    }
}

void MapCanvasWidget::mouseMoveEvent(QMouseEvent* event) {
    int cx, cy;
    // buttons() (no button()): durante el drag el boton ya esta apretado.
    if (doc_ && (event->buttons() & Qt::LeftButton) &&
        cell_at(event->pos(), &cx, &cy)) {
        doc_->apply_tool_drag(cx, cy);
    }
}

void MapCanvasWidget::mouseReleaseEvent(QMouseEvent* event) {
    int cx, cy;
    if (doc_ && event->button() == Qt::LeftButton) {
        // Cierra el gesto aunque se suelte fuera del grid (no pinta esa celda,
        // pero apila el Command con lo ya pintado).
        if (!cell_at(event->pos(), &cx, &cy)) { cx = -1; cy = -1; }
        doc_->apply_tool_release(cx, cy);
    }
}

void MapCanvasWidget::paintEvent(QPaintEvent* /*event*/) {
    QPainter painter(this);

    const int ts = tile_px();
    const QPoint o = origin();
    const int map_w = WIDTH * ts;
    const int map_h = HEIGHT * ts;

    // Fondo: todo el widget gris oscuro, area del mapa negra.
    painter.fillRect(rect(), QColor(30, 30, 30));
    painter.fillRect(QRect(o.x(), o.y(), map_w, map_h), QColor(0, 0, 0));

    // Tiles del mapa, capa por capa (algoritmo del pintor). Los teleports NO son
    // una capa: se dibujan aparte como borde amarillo, mas abajo.
    if (doc_) {
        const Map& map = doc_->map();
        for (int layer = 0; layer < map.layer_count(); ++layer) {
            for (int y = 0; y < HEIGHT; ++y) {
                for (int x = 0; x < WIDTH; ++x) {
                    int gid = map.get_cell(layer, x, y);
                    if (gid == 0) continue;  // celda vacia

                    const TileDef* td = map.find_tile(gid);
                    if (!td) continue;
                    const QPixmap* sheet = cache_.pixmap_for_tileset(td->tileset_index);
                    QRect dst(o.x() + x * ts, o.y() + y * ts, ts, ts);

                    if (sheet && !sheet->isNull()) {
                        painter.drawPixmap(dst, *sheet, cache_.src_rect(map, gid));
                    } else {
                        // Fallback de color si el PNG no cargo: color por gid.
                        painter.fillRect(dst, QColor::fromHsv((gid * 47) % 360, 160, 200));
                    }
                }
            }
        }
    }
    // Borde amarillo en cada celda teleport (decoracion SOLO del editor; el dato
    // vive en map.teleports(), no en una capa de tiles).
    if (doc_) {
        const Map& map = doc_->map();
        QPen tp_pen(QColor(255, 230, 0), 3);  // amarillo, 3px
        painter.setPen(tp_pen);
        painter.setBrush(Qt::NoBrush);
        for (const TeleportDef& tp : map.teleports()) {
            QRect r(o.x() + tp.x * ts + 1, o.y() + tp.y * ts + 1, ts - 2, ts - 2);
            painter.drawRect(r);
        }
    }

    // Overlay de colision: relleno rojo semitransparente sobre cada celda
    // bloqueada (decoracion SOLO del editor; el dato vive en map.collision()).
    if (doc_) {
        const Map& map = doc_->map();
        const QColor block_color(220, 40, 40, 110);  // rojo, alpha ~43%
        for (int y = 0; y < HEIGHT; ++y) {
            for (int x = 0; x < WIDTH; ++x) {
                if (!map.is_blocked_cell(x, y)) continue;
                QRect dst(o.x() + x * ts, o.y() + y * ts, ts, ts);
                painter.fillRect(dst, block_color);
            }
        }
    }

    // Grilla: lineas verticales y horizontales cada tile.
    painter.setPen(QColor(70, 70, 70));
    for (int col = 0; col <= WIDTH; ++col) {
        int x = o.x() + col * ts;
        painter.drawLine(x, o.y(), x, o.y() + map_h);
    }
    for (int row = 0; row <= HEIGHT; ++row) {
        int y = o.y() + row * ts;
        painter.drawLine(o.x(), y, o.x() + map_w, y);
    }
}
