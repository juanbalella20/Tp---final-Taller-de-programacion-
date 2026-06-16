#ifndef EDITOR_RENDER_MAPEDITORSCENE_H_
#define EDITOR_RENDER_MAPEDITORSCENE_H_

#include <QGraphicsScene>
#include <QHash>

#include <vector>

#include "../document/TileBrush.h"

class EditorDocument;
class QGraphicsPixmapItem;
class QGraphicsSceneDragDropEvent;
class QGraphicsSceneMouseEvent;
class QPainter;

// Canvas del editor.
class MapEditorScene : public QGraphicsScene {
  Q_OBJECT

public:
  // Crea los items de cada celda y se conecta a las senales de 'document'.
  explicit MapEditorScene(EditorDocument *document, QObject *parent = nullptr);

  // Cantidad total de items visuales (suma de todas las capas). Para tests.
  int visualItemCount() const;
  // Item de la celda (column,row) en 'layer'; nullptr si esta fuera de rango.
  QGraphicsPixmapItem *visualItem(int layer, int column, int row) const;

public slots:
  // Rearma la cache de pixmaps gid->tile desde los tilesets y repinta todo.
  void rebuildTileCache();
  // Refresca el pixmap de todas las celdas de todas las capas.
  void refreshAll();

protected:
  // Pinta el fondo negro y la grilla de la escena.
  void drawBackground(QPainter *painter, const QRectF &rect) override;
  // Pinta las superposiciones de colision y teleports sobre los tiles.
  void drawForeground(QPainter *painter, const QRectF &rect) override;
  // Inicia/continua/cierra un gesto de herramienta segun el boton del mouse
  // (izquierdo = herramienta activa, derecho = goma).
  void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
  void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
  void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
  // Recibe bloques arrastrados desde la paleta de tilesets.
  void dragEnterEvent(QGraphicsSceneDragDropEvent *event) override;
  void dragMoveEvent(QGraphicsSceneDragDropEvent *event) override;
  // Se llama si el drag sale del canvas sin soltar
  void dragLeaveEvent(QGraphicsSceneDragDropEvent *event) override;
  // Se llama cuando se suelta el mouse sobre el canvas
  void dropEvent(QGraphicsSceneDragDropEvent *event) override;

private slots:
  // Repinta una sola celda cuando el documento avisa que cambio.
  void handleCellChanged(int layer, int column, int row);

private:
  static constexpr int kPreviewPenWidth = 1;

  EditorDocument *m_document = nullptr;
  // Items visuales indexados como [capa][celda].
  std::vector<std::vector<QGraphicsPixmapItem *>> m_visualItems;
  QHash<int, QPixmap> m_tileCache;                // gid -> pixmap del tile
  Qt::MouseButton m_gestureButton = Qt::NoButton; // boton del gesto en curso
  // guarda el rectangulo azul que se dibuja sobre el mapa antes de soltar
  QRectF m_dropPreviewRect;
  // guarda temporalmente el bloque que viene desde la paleta
  TileBrush m_pendingDropBrush;

  // Crea un QGraphicsPixmapItem vacio por cada celda de cada capa.
  void initializeVisualItems();
  // Carga en el item de (column,row) el pixmap del gid de esa celda.
  void updateVisualItem(int layer, int column, int row);
  // Traduce una posicion de escena a celda (column,row); false si cae afuera.
  bool cellAt(const QPointF &scenePosition, int *column, int *row) const;
  // Rectangulo en coordenadas de escena que ocupa la celda (column,row).
  QRectF cellRect(int column, int row) const;
  // Llama a pintar y despintar el rectangulo de preview
  // al arrastrar uno o varios tiles sobre el canvas.
  void setDropPreviewRect(const QRectF &rect);
};

#endif // EDITOR_RENDER_MAPEDITORSCENE_H_
