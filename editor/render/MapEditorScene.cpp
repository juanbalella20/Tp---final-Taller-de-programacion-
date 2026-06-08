#include "MapEditorScene.h"

#include <QColor>
#include <QGraphicsPixmapItem>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QPen>

#include <algorithm>
#include <cmath>

#include "../document/EditorDocument.h"
#include "../model/Map.h"
#include "../tools/ToolType.h"
#include "TileLibrary.h"

namespace {

QPixmap fallbackPixmap(int gid, int tileSize) {
    QPixmap pixmap(tileSize, tileSize);
    pixmap.fill(QColor::fromHsv((gid * 47) % 360, 160, 200));
    return pixmap;
}

}  // namespace

MapEditorScene::MapEditorScene(EditorDocument* document, QObject* parent)
    : QGraphicsScene(parent), m_document(document) {
    if (!m_document) return;

    const Map& map = m_document->map();
    setSceneRect(0.0, 0.0, map.width() * map.tile_size(),
                 map.height() * map.tile_size());
    initializeVisualItems();

    connect(m_document, &EditorDocument::cellChanged, this,
            &MapEditorScene::handleCellChanged);
    connect(m_document, &EditorDocument::mapReset, this,
            &MapEditorScene::refreshAll);
    connect(m_document, &EditorDocument::tilesetsChanged, this,
            &MapEditorScene::rebuildTileCache);

    rebuildTileCache();
}

int MapEditorScene::visualItemCount() const {
    int count = 0;
    for (const auto& layer : m_visualItems) {
        count += static_cast<int>(layer.size());
    }
    return count;
}

QGraphicsPixmapItem* MapEditorScene::visualItem(int layer, int column,
                                                int row) const {
    if (!m_document) return nullptr;
    const Map& map = m_document->map();
    if (layer < 0 || layer >= static_cast<int>(m_visualItems.size()) ||
        !map.in_bounds(column, row)) {
        return nullptr;
    }
    const int index = row * map.width() + column;
    return m_visualItems[static_cast<std::size_t>(layer)]
                        [static_cast<std::size_t>(index)];
}

void MapEditorScene::initializeVisualItems() {
    const Map& map = m_document->map();
    m_visualItems.resize(static_cast<std::size_t>(map.layer_count()));

    for (int layer = 0; layer < map.layer_count(); ++layer) {
        auto& layerItems = m_visualItems[static_cast<std::size_t>(layer)];
        layerItems.reserve(static_cast<std::size_t>(map.width() * map.height()));
        for (int row = 0; row < map.height(); ++row) {
            for (int column = 0; column < map.width(); ++column) {
                auto* item = addPixmap(QPixmap());
                item->setPos(column * map.tile_size(), row * map.tile_size());
                item->setZValue(layer);
                item->setTransformationMode(Qt::FastTransformation);
                layerItems.push_back(item);
            }
        }
    }
}

void MapEditorScene::rebuildTileCache() {
    if (!m_document) return;
    m_tileCache.clear();

    const Map& map = m_document->map();
    for (const Tileset& tileset : map.tilesets()) {
        TileLibrary library;
        const bool loaded = library.loadTileset(
            QString::fromStdString(tileset.file_path), map.tile_size());

        for (int localId = 0; localId < tileset.tile_count; ++localId) {
            const int gid = tileset.firstgid + localId;
            if (loaded && localId < library.tileCount()) {
                m_tileCache.insert(gid, library.getTile(localId));
            } else {
                m_tileCache.insert(gid, fallbackPixmap(gid, map.tile_size()));
            }
        }
    }

    refreshAll();
}

void MapEditorScene::refreshAll() {
    if (!m_document) return;
    const Map& map = m_document->map();
    for (int layer = 0; layer < map.layer_count(); ++layer) {
        for (int row = 0; row < map.height(); ++row) {
            for (int column = 0; column < map.width(); ++column) {
                updateVisualItem(layer, column, row);
            }
        }
    }
    update();
}

void MapEditorScene::updateVisualItem(int layer, int column, int row) {
    QGraphicsPixmapItem* item = visualItem(layer, column, row);
    if (!item) return;

    const int gid = m_document->map().get_cell(layer, column, row);
    if (gid == 0) {
        item->setPixmap(QPixmap());
        return;
    }

    const auto cached = m_tileCache.constFind(gid);
    item->setPixmap(cached == m_tileCache.cend() ? QPixmap() : cached.value());
}

void MapEditorScene::handleCellChanged(int layer, int column, int row) {
    if (!m_document || !m_document->map().in_bounds(column, row)) return;
    if (layer >= 0) updateVisualItem(layer, column, row);
    update(cellRect(column, row));
}

void MapEditorScene::drawBackground(QPainter* painter, const QRectF& rect) {
    if (!m_document) return;
    const Map& map = m_document->map();
    const QRectF mapRect = sceneRect();

    painter->fillRect(rect, QColor(0, 0, 0));
    QPen gridPen(QColor(70, 70, 70));
    gridPen.setWidth(0);
    painter->setPen(gridPen);

    for (int column = 0; column <= map.width(); ++column) {
        const qreal x = column * map.tile_size();
        painter->drawLine(QPointF(x, mapRect.top()),
                          QPointF(x, mapRect.bottom()));
    }
    for (int row = 0; row <= map.height(); ++row) {
        const qreal y = row * map.tile_size();
        painter->drawLine(QPointF(mapRect.left(), y),
                          QPointF(mapRect.right(), y));
    }
}

void MapEditorScene::drawForeground(QPainter* painter, const QRectF& rect) {
    if (!m_document) return;
    const Map& map = m_document->map();
    const int tileSize = map.tile_size();

    const int firstColumn =
        std::max(0, static_cast<int>(std::floor(rect.left() / tileSize)));
    const int lastColumn = std::min(
        map.width() - 1,
        static_cast<int>(std::floor(rect.right() / tileSize)));
    const int firstRow =
        std::max(0, static_cast<int>(std::floor(rect.top() / tileSize)));
    const int lastRow = std::min(
        map.height() - 1,
        static_cast<int>(std::floor(rect.bottom() / tileSize)));

    painter->save();
    painter->setPen(Qt::NoPen);
    painter->setBrush(QColor(220, 40, 40, 110));
    for (int row = firstRow; row <= lastRow; ++row) {
        for (int column = firstColumn; column <= lastColumn; ++column) {
            if (map.is_blocked_cell(column, row)) {
                painter->drawRect(cellRect(column, row));
            }
        }
    }

    QPen teleportPen(QColor(255, 230, 0), 3);
    painter->setPen(teleportPen);
    painter->setBrush(Qt::NoBrush);
    for (const TeleportDef& teleport : map.teleports()) {
        const QRectF teleportRect(
            teleport.x * tileSize + 1, teleport.y * tileSize + 1,
            tileSize - 2, tileSize - 2);
        if (rect.intersects(teleportRect)) painter->drawRect(teleportRect);
    }
    painter->restore();
}

bool MapEditorScene::cellAt(const QPointF& scenePosition, int* column,
                            int* row) const {
    if (!m_document || scenePosition.x() < 0.0 || scenePosition.y() < 0.0) {
        return false;
    }

    const Map& map = m_document->map();
    const int computedColumn =
        static_cast<int>(std::floor(scenePosition.x() / map.tile_size()));
    const int computedRow =
        static_cast<int>(std::floor(scenePosition.y() / map.tile_size()));
    if (!map.in_bounds(computedColumn, computedRow)) return false;

    *column = computedColumn;
    *row = computedRow;
    return true;
}

QRectF MapEditorScene::cellRect(int column, int row) const {
    const int tileSize = m_document->map().tile_size();
    return QRectF(column * tileSize, row * tileSize, tileSize, tileSize);
}

void MapEditorScene::mousePressEvent(QGraphicsSceneMouseEvent* event) {
    int column = 0;
    int row = 0;
    if (!cellAt(event->scenePos(), &column, &row) ||
        (event->button() != Qt::LeftButton &&
         event->button() != Qt::RightButton)) {
        QGraphicsScene::mousePressEvent(event);
        return;
    }

    m_gestureButton = event->button();
    if (m_gestureButton == Qt::RightButton) {
        m_document->apply_tool_press(column, row, ToolType::Eraser);
    } else {
        m_document->apply_tool_press(column, row);
    }
    event->accept();
}

void MapEditorScene::mouseMoveEvent(QGraphicsSceneMouseEvent* event) {
    if (m_gestureButton == Qt::NoButton ||
        !(event->buttons() & m_gestureButton)) {
        QGraphicsScene::mouseMoveEvent(event);
        return;
    }

    int column = 0;
    int row = 0;
    if (cellAt(event->scenePos(), &column, &row)) {
        m_document->apply_tool_drag(column, row);
    }
    event->accept();
}

void MapEditorScene::mouseReleaseEvent(QGraphicsSceneMouseEvent* event) {
    if (m_gestureButton == Qt::NoButton ||
        event->button() != m_gestureButton) {
        QGraphicsScene::mouseReleaseEvent(event);
        return;
    }

    m_document->apply_tool_release(-1, -1);
    m_gestureButton = Qt::NoButton;
    event->accept();
}
