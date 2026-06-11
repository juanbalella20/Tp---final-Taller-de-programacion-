#ifndef EDITOR_RENDER_MAPEDITORSCENE_H_
#define EDITOR_RENDER_MAPEDITORSCENE_H_

#include <QGraphicsScene>
#include <QHash>

#include <vector>

class EditorDocument;
class QGraphicsPixmapItem;
class QGraphicsSceneMouseEvent;
class QPainter;

/*
 * Canvas del editor. Dibuja el Map como una grilla de QGraphicsPixmapItem (uno
 * por celda y por capa) sobre un fondo cuadriculado, mas las superposiciones de
 * colision (rojo) y teleports (borde amarillo). Traduce los gestos del mouse en
 * llamadas a las herramientas del EditorDocument y se repinta escuchando sus
 * senales (cellChanged / mapReset / tilesetsChanged).
 */
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

private slots:
  // Repinta una sola celda cuando el documento avisa que cambio.
  void handleCellChanged(int layer, int column, int row);

private:
  EditorDocument *m_document = nullptr;
  std::vector<std::vector<QGraphicsPixmapItem *>> m_visualItems; // [capa][celda]
  QHash<int, QPixmap> m_tileCache;                // gid -> pixmap del tile
  Qt::MouseButton m_gestureButton = Qt::NoButton; // boton del gesto en curso

  // Crea un QGraphicsPixmapItem vacio por cada celda de cada capa.
  void initializeVisualItems();
  // Carga en el item de (column,row) el pixmap del gid de esa celda.
  void updateVisualItem(int layer, int column, int row);
  // Traduce una posicion de escena a celda (column,row); false si cae afuera.
  bool cellAt(const QPointF &scenePosition, int *column, int *row) const;
  // Rectangulo en coordenadas de escena que ocupa la celda (column,row).
  QRectF cellRect(int column, int row) const;
};

#endif // EDITOR_RENDER_MAPEDITORSCENE_H_
