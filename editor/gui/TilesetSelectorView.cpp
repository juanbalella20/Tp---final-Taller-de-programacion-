#include "TilesetSelectorView.h"

#include <QApplication>
#include <QDrag>
#include <QGraphicsPixmapItem>
#include <QGraphicsScene>
#include <QMimeData>
#include <QMouseEvent>
#include <QPainter>
#include <QPen>

#include <algorithm>
#include <cmath>
#include <utility>

#include "TileBrushMime.h"

TilesetSelectorView::TilesetSelectorView(QWidget *parent)
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

bool TilesetSelectorView::setTileset(const QString &path, int tileSize,
                                     int firstGid) {
  TileLibrary library;
  if (!library.loadTileset(path, tileSize))
    return false;

  m_library = std::move(library);
  m_firstGid = firstGid;
  m_selectedLocalId = -1;
  resetSelection();
  m_sheetItem->setPixmap(m_library.masterPixmap());
  m_sheetScene->setSceneRect(m_library.masterPixmap().rect());
  viewport()->update();
  return true;
}

void TilesetSelectorView::clearTileset() {
  m_library.clear();
  m_firstGid = 0;
  m_selectedLocalId = -1;
  resetSelection();
  m_sheetItem->setPixmap(QPixmap());
  m_sheetScene->setSceneRect(QRectF());
  viewport()->update();
}

int TilesetSelectorView::tileIdAt(const QPointF &scenePosition) const {
  if (m_library.tileSize() <= 0 || scenePosition.x() < 0.0 ||
      scenePosition.y() < 0.0 || scenePosition.x() >= m_library.usableWidth() ||
      scenePosition.y() >= m_library.usableHeight()) {
    return -1;
  }

  const int column =
      static_cast<int>(std::floor(scenePosition.x() / m_library.tileSize()));
  const int row =
      static_cast<int>(std::floor(scenePosition.y() / m_library.tileSize()));
  const int localId = row * m_library.columns() + column;
  if (localId < 0 || localId >= m_library.tileCount())
    return -1;
  return m_firstGid + localId;
}

QPoint TilesetSelectorView::tileCellAt(const QPointF &scenePosition) const {
  if (m_library.tileSize() <= 0 || scenePosition.x() < 0.0 ||
      scenePosition.y() < 0.0 || scenePosition.x() >= m_library.usableWidth() ||
      scenePosition.y() >= m_library.usableHeight())
    return QPoint(OUT_OF_RANGE, OUT_OF_RANGE);

  const int column =
      static_cast<int>(std::floor(scenePosition.x() / m_library.tileSize()));
  const int row =
      static_cast<int>(std::floor(scenePosition.y() / m_library.tileSize()));
  if (row * m_library.columns() + column >= m_library.tileCount())
    return QPoint(OUT_OF_RANGE, OUT_OF_RANGE);
  return QPoint(column, row);
}

QRect TilesetSelectorView::normalizedSelection() const {
  const int left = std::min(m_selectionStart.x(), m_selectionEnd.x());
  const int right = std::max(m_selectionStart.x(), m_selectionEnd.x());
  const int top = std::min(m_selectionStart.y(), m_selectionEnd.y());
  const int bottom = std::max(m_selectionStart.y(), m_selectionEnd.y());
  return QRect(left, top, right - left + 1, bottom - top + 1);
}

void TilesetSelectorView::resetSelection() {
  m_selectionStart = m_selectionEnd = QPoint(-1, -1);
  m_selecting = false;
  m_dragPending = false;
}

TileBrush TilesetSelectorView::selectedBrush() const {
  TileBrush brush;
  if (m_selectionStart.x() < 0 || m_selectionStart.y() < 0 ||
      m_selectionEnd.x() < 0 || m_selectionEnd.y() < 0 ||
      m_library.columns() <= 0)
    return brush;

  const QRect sel = normalizedSelection();
  brush.width = sel.width();
  brush.height = sel.height();
  brush.gids.reserve(static_cast<std::size_t>(brush.width * brush.height));

  for (int row = sel.top(); row <= sel.bottom(); ++row) {
    for (int column = sel.left(); column <= sel.right(); ++column) {
      // gid global = primer gid del tileset + índice local dentro del
      // spritesheet
      brush.gids.push_back(m_firstGid + row * m_library.columns() + column);
    }
  }
  return brush;
}

QRectF TilesetSelectorView::selectionRect() const {
  if (m_selectionStart.x() < 0 || m_selectionStart.y() < 0 ||
      m_selectionEnd.x() < 0 || m_selectionEnd.y() < 0)
    return QRectF();

  const QRect sel = normalizedSelection();
  const int tileSize = m_library.tileSize();
  return QRectF(sel.left() * tileSize, sel.top() * tileSize,
                sel.width() * tileSize, sel.height() * tileSize);
}

void TilesetSelectorView::startBrushDrag() {
  const TileBrush brush = selectedBrush();
  const QRectF selected = selectionRect();
  if (brush.gids.empty() || selected.isEmpty())
    return;

  auto *mimeData = new QMimeData;
  mimeData->setData(TILE_BRUSH_MIME_TYPE, encode_tile_brush(brush));

  QDrag drag(this);
  drag.setMimeData(mimeData);
  // Define la imagen que se ve mientras se arrastra:
  // spritesheet completo recortado por la seleccion actual
  drag.setPixmap(m_library.masterPixmap().copy(selected.toAlignedRect()));

  const QPointF pressScene = mapToScene(m_dragStartPosition);
  // hotspot = punto de imagen que se seleccionó con el cursor
  drag.setHotSpot((pressScene - selected.topLeft()).toPoint());
  // arranca drag and drop
  drag.exec(Qt::CopyAction);
}

void TilesetSelectorView::drawForeground(QPainter *painter,
                                         const QRectF & /*rect*/) {
  if (m_library.tileSize() <= 0)
    return;

  painter->save();
  QPen gridPen(QColor(255, 255, 255, 155));
  gridPen.setWidth(0);
  painter->setPen(gridPen);
  painter->setBrush(Qt::NoBrush);

  for (int column = 0; column <= m_library.columns(); ++column) {
    const qreal x = column * m_library.tileSize();
    painter->drawLine(QPointF(x, 0.0), QPointF(x, m_library.usableHeight()));
  }
  for (int row = 0; row <= m_library.rows(); ++row) {
    const qreal y = row * m_library.tileSize();
    painter->drawLine(QPointF(0.0, y), QPointF(m_library.usableWidth(), y));
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

void TilesetSelectorView::mousePressEvent(QMouseEvent *event) {
  if (event->button() != Qt::LeftButton) {
    QGraphicsView::mousePressEvent(event);
    return;
  }

  const QPointF scenePosition = mapToScene(event->position().toPoint());
  const int gid = tileIdAt(scenePosition);
  const QPoint cell = tileCellAt(scenePosition);
  if (gid < m_firstGid || cell.x() < 0) {
    m_dragPending = false;
    event->accept();
    return;
  }

  // Si no se está usando el Shift y si ya se seleccionó un bloque,
  // se habilita drag and drop.
  if (!event->modifiers().testFlag(Qt::ShiftModifier) &&
      selectionRect().contains(scenePosition)) {
    m_dragStartPosition = event->position().toPoint();
    m_dragPending = true;
    event->accept();
    return;
  }

  // Caso shift+click
  m_selectedLocalId = gid - m_firstGid;
  m_selectionStart = cell;
  m_selectionEnd = cell;
  m_selecting = event->modifiers().testFlag(Qt::ShiftModifier);
  m_dragStartPosition = event->position().toPoint();
  m_dragPending = !m_selecting;
  viewport()->update();
  emit tileSelected(gid, m_library.getTile(m_selectedLocalId));
  if (!m_selecting)
    emit brushSelected(selectedBrush());
  event->accept();
}

void TilesetSelectorView::mouseMoveEvent(QMouseEvent *event) {
  if (m_selecting && (event->buttons() & Qt::LeftButton)) {
    const QPoint cell = tileCellAt(mapToScene(event->position().toPoint()));
    if (cell.x() >= 0) {
      m_selectionEnd = cell;
      viewport()->update();
    }
    event->accept();
    return;
  }

  // Hay un drag pendiente
  if (m_dragPending && (event->buttons() & Qt::LeftButton) &&
      (event->position().toPoint() - m_dragStartPosition).manhattanLength() >=
          QApplication::startDragDistance()) {
    m_dragPending = false;
    startBrushDrag();
    event->accept();
    return;
  }

  if (!m_selecting) {
    QGraphicsView::mouseMoveEvent(event);
    return;
  }

  event->accept();
}

void TilesetSelectorView::mouseReleaseEvent(QMouseEvent *event) {
  if (!m_selecting || event->button() != Qt::LeftButton) {
    if (event->button() == Qt::LeftButton)
      m_dragPending = false;
    QGraphicsView::mouseReleaseEvent(event);
    return;
  }

  const QPoint cell = tileCellAt(mapToScene(event->position().toPoint()));
  if (cell.x() >= 0)
    m_selectionEnd = cell;

  m_selecting = false;
  viewport()->update();
  emit brushSelected(selectedBrush());
  event->accept();
}
