#ifndef EDITOR_RENDER_MAPEDITORSCENE_H_
#define EDITOR_RENDER_MAPEDITORSCENE_H_

#include <QGraphicsScene>
#include <QHash>

#include <vector>

class EditorDocument;
class QGraphicsPixmapItem;
class QGraphicsSceneMouseEvent;
class QPainter;

class MapEditorScene : public QGraphicsScene {
  Q_OBJECT

public:
  explicit MapEditorScene(EditorDocument *document, QObject *parent = nullptr);

  int visualItemCount() const;
  QGraphicsPixmapItem *visualItem(int layer, int column, int row) const;

public slots:
  void rebuildTileCache();
  void refreshAll();

protected:
  void drawBackground(QPainter *painter, const QRectF &rect) override;
  void drawForeground(QPainter *painter, const QRectF &rect) override;
  void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
  void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
  void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;

private slots:
  void handleCellChanged(int layer, int column, int row);

private:
  EditorDocument *m_document = nullptr;
  std::vector<std::vector<QGraphicsPixmapItem *>> m_visualItems;
  QHash<int, QPixmap> m_tileCache;
  Qt::MouseButton m_gestureButton = Qt::NoButton;

  void initializeVisualItems();
  void updateVisualItem(int layer, int column, int row);
  bool cellAt(const QPointF &scenePosition, int *column, int *row) const;
  QRectF cellRect(int column, int row) const;
};

#endif // EDITOR_RENDER_MAPEDITORSCENE_H_
