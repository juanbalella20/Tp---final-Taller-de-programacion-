#ifndef EDITOR_GUI_TILESETSELECTORVIEW_H_
#define EDITOR_GUI_TILESETSELECTORVIEW_H_

#include <QGraphicsView>
#include <QPoint>
#include <QRect>
#include <QRectF>

#include "../document/TileBrush.h"
#include "../render/TileLibrary.h"

#define OUT_OF_RANGE -1

class QGraphicsPixmapItem;
class QGraphicsScene;
class QMouseEvent;
class QPainter;

// Paleta de tiles. Muestra el spritesheet de un tileset.
class TilesetSelectorView : public QGraphicsView {
  Q_OBJECT

public:
  explicit TilesetSelectorView(QWidget *parent = nullptr);

  // Carga el PNG de 'path' como tileset activo; 'firstGid' es el gid del primer
  // tile. Devuelve false si no se pudo cargar.
  bool setTileset(const QString &path, int tileSize, int firstGid);
  // Vacia la paleta.
  void clearTileset();

  // gid del tile en la posicion de escena dada; -1 si cae fuera de la grilla.
  int tileIdAt(const QPointF &scenePosition) const;
  // gid del tile seleccionado; 0 si no hay seleccion.
  int selectedGid() const;
  // Convierte la selección visual actual en un TileBrush.
  TileBrush selectedBrush() const;
  // Rectangulo del tile seleccionado; vacio si no hay seleccion.
  QRectF selectionRect() const;
  // Acceso de lectura al tileset cargado.
  const TileLibrary &tileLibrary() const;

signals:
  // Emitida al elegir un tile: gid global y su pixmap.
  void tileSelected(int gid, QPixmap pixmap);
  // Emitida al completar una seleccion simple o rectangular.
  void brushSelected(TileBrush brush);

protected:
  // Dibuja la grilla y el resaltado de la seleccion sobre el spritesheet.
  void drawForeground(QPainter *painter, const QRectF &rect) override;
  // Click simple selecciona un tile. Shift + arrastre selecciona un rectangulo.
  void mousePressEvent(QMouseEvent *event) override;
  void mouseMoveEvent(QMouseEvent *event) override;
  void mouseReleaseEvent(QMouseEvent *event) override;

private:
  // Convierte una posición en píxeles dentro del spritesheet a una celda
  // (columna, fila).
  QPoint tileCellAt(const QPointF &scenePosition) const;
  // Arregla el rectángulo si el usuario arrastra al revés.
  QRect normalizedSelection() const;
  // Resetea el estado de seleccion y drag.
  void resetSelection();
  // Convierte la selección actual y la prepara para el drag-and-drop.
  void startBrushDrag();

  QGraphicsScene *m_sheetScene = nullptr;
  QGraphicsPixmapItem *m_sheetItem = nullptr;
  TileLibrary m_library;
  int m_firstGid = 0;
  int m_selectedLocalId = -1; // -1 = sin seleccion
  QPoint m_selectionStart{-1, -1};
  QPoint m_selectionEnd{-1, -1};
  bool m_selecting = false;
  QPoint m_dragStartPosition;
  bool m_dragPending = false;
};

#endif // EDITOR_GUI_TILESETSELECTORVIEW_H_
