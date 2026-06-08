#include "TilesetSelectorView.h"

#include <QGraphicsPixmapItem>
#include <QGraphicsScene>
#include <QMouseEvent>
#include <QPainter>
#include <QPen>

#include <cmath>
#include <utility>

TilesetSelectorView::TilesetSelectorView(QWidget* parent)
    : QGraphicsView(parent), m_sheetScene(new QGraphicsScene(this)) {
    setScene(m_sheetScene);
    m_sheetItem = m_sheetScene->addPixmap(QPixmap());

    setAlignment(Qt::AlignLeft | Qt::AlignTop);
    setDragMode(QGraphicsView::NoDrag);
    setInteractive(false);
    setOptimizationFlag(QGraphicsView::DontAdjustForAntialiasing, true);
    setRenderHint(QPainter::Antialiasing, false);
    setBackgroundBrush(QColor(24, 24, 24));
}

bool TilesetSelectorView::setTileset(const QString& path, int tileSize,
                                     int firstGid) {
    TileLibrary library;
    if (!library.loadTileset(path, tileSize)) return false;

    m_library = std::move(library);
    m_firstGid = firstGid;
    m_selectedLocalId = -1;
    m_sheetItem->setPixmap(m_library.masterPixmap());
    m_sheetScene->setSceneRect(m_library.masterPixmap().rect());
    viewport()->update();
    return true;
}

void TilesetSelectorView::clearTileset() {
    m_library.clear();
    m_firstGid = 0;
    m_selectedLocalId = -1;
    m_sheetItem->setPixmap(QPixmap());
    m_sheetScene->setSceneRect(QRectF());
    viewport()->update();
}

int TilesetSelectorView::tileIdAt(const QPointF& scenePosition) const {
    if (m_library.tileSize() <= 0 || scenePosition.x() < 0.0 ||
        scenePosition.y() < 0.0 ||
        scenePosition.x() >= m_library.usableWidth() ||
        scenePosition.y() >= m_library.usableHeight()) {
        return -1;
    }

    const int column =
        static_cast<int>(std::floor(scenePosition.x() / m_library.tileSize()));
    const int row =
        static_cast<int>(std::floor(scenePosition.y() / m_library.tileSize()));
    const int localId = row * m_library.columns() + column;
    if (localId < 0 || localId >= m_library.tileCount()) return -1;
    return m_firstGid + localId;
}

int TilesetSelectorView::selectedGid() const {
    if (m_selectedLocalId < 0) return 0;
    return m_firstGid + m_selectedLocalId;
}

QRectF TilesetSelectorView::selectionRect() const {
    if (m_selectedLocalId < 0 || m_library.columns() <= 0) return QRectF();
    const int column = m_selectedLocalId % m_library.columns();
    const int row = m_selectedLocalId / m_library.columns();
    const int tileSize = m_library.tileSize();
    return QRectF(column * tileSize, row * tileSize, tileSize, tileSize);
}

const TileLibrary& TilesetSelectorView::tileLibrary() const {
    return m_library;
}

void TilesetSelectorView::drawForeground(QPainter* painter,
                                         const QRectF& /*rect*/) {
    if (m_library.tileSize() <= 0) return;

    painter->save();
    QPen gridPen(QColor(255, 255, 255, 155));
    gridPen.setWidth(0);
    painter->setPen(gridPen);
    painter->setBrush(Qt::NoBrush);

    for (int column = 0; column <= m_library.columns(); ++column) {
        const qreal x = column * m_library.tileSize();
        painter->drawLine(QPointF(x, 0.0),
                          QPointF(x, m_library.usableHeight()));
    }
    for (int row = 0; row <= m_library.rows(); ++row) {
        const qreal y = row * m_library.tileSize();
        painter->drawLine(QPointF(0.0, y),
                          QPointF(m_library.usableWidth(), y));
    }

    const QRectF selected = selectionRect();
    if (!selected.isEmpty()) {
        painter->fillRect(selected, QColor(40, 160, 255, 80));
        QPen selectionPen(QColor(80, 190, 255, 230));
        selectionPen.setWidth(0);
        painter->setPen(selectionPen);
        painter->drawRect(selected);
    }
    painter->restore();
}

void TilesetSelectorView::mousePressEvent(QMouseEvent* event) {
    if (event->button() != Qt::LeftButton) {
        QGraphicsView::mousePressEvent(event);
        return;
    }

    const QPointF scenePosition = mapToScene(event->position().toPoint());
    const int gid = tileIdAt(scenePosition);
    if (gid < m_firstGid) {
        event->accept();
        return;
    }

    m_selectedLocalId = gid - m_firstGid;
    viewport()->update();
    emit tileSelected(gid, m_library.getTile(m_selectedLocalId));
    event->accept();
}
