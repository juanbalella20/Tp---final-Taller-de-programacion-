#ifndef EDITOR_TILESET_SELECTOR_VIEW_H
#define EDITOR_TILESET_SELECTOR_VIEW_H

#include <QGraphicsView>
#include <QRectF>

#include "../render/TileLibrary.h"

class QGraphicsPixmapItem;
class QGraphicsScene;
class QMouseEvent;
class QPainter;

class TilesetSelectorView : public QGraphicsView {
  Q_OBJECT

public:
  explicit TilesetSelectorView(QWidget *parent = nullptr);

  bool setTileset(const QString &path, int tileSize, int firstGid);
  void clearTileset();

  int tileIdAt(const QPointF &scenePosition) const;
  int selectedGid() const;
  QRectF selectionRect() const;
  const TileLibrary &tileLibrary() const;

signals:
  void tileSelected(int gid, QPixmap pixmap);

protected:
  void drawForeground(QPainter *painter, const QRectF &rect) override;
  void mousePressEvent(QMouseEvent *event) override;

private:
  QGraphicsScene *m_sheetScene = nullptr;
  QGraphicsPixmapItem *m_sheetItem = nullptr;
  TileLibrary m_library;
  int m_firstGid = 0;
  int m_selectedLocalId = -1;
};

#endif // EDITOR_TILESET_SELECTOR_VIEW_H
