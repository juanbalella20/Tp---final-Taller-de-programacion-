#ifndef EDITOR_GUI_TILESETSELECTORVIEW_H_
#define EDITOR_GUI_TILESETSELECTORVIEW_H_

#include <QGraphicsView>
#include <QRectF>

#include "../render/TileLibrary.h"

class QGraphicsPixmapItem;
class QGraphicsScene;
class QMouseEvent;
class QPainter;

/*
 * Paleta de tiles. Muestra el spritesheet de un tileset con una grilla
 * superpuesta y deja elegir un tile con el mouse. Al seleccionar emite
 * tileSelected(gid, pixmap), que la ventana usa para fijar el brush activo.
 */
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
  // Rectangulo del tile seleccionado; vacio si no hay seleccion.
  QRectF selectionRect() const;
  // Acceso de lectura al tileset cargado.
  const TileLibrary &tileLibrary() const;

signals:
  // Emitida al elegir un tile: gid global y su pixmap.
  void tileSelected(int gid, QPixmap pixmap);

protected:
  // Dibuja la grilla y el resaltado de la seleccion sobre el spritesheet.
  void drawForeground(QPainter *painter, const QRectF &rect) override;
  // Selecciona el tile clickeado (boton izquierdo) y emite tileSelected.
  void mousePressEvent(QMouseEvent *event) override;

private:
  QGraphicsScene *m_sheetScene = nullptr;
  QGraphicsPixmapItem *m_sheetItem = nullptr;
  TileLibrary m_library;
  int m_firstGid = 0;
  int m_selectedLocalId = -1; // -1 = sin seleccion
};

#endif // EDITOR_GUI_TILESETSELECTORVIEW_H_
