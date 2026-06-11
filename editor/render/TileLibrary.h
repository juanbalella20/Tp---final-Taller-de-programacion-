#ifndef EDITOR_RENDER_TILELIBRARY_H_
#define EDITOR_RENDER_TILELIBRARY_H_

#include <QPixmap>
#include <QString>

#include <vector>

/*
 * Carga un PNG de tileset y lo trocea en pixmaps de un tile cada uno, indexados
 * por su id local (0..tileCount-1). La usan la paleta (TilesetSelectorView) y el
 * canvas (MapEditorScene) para dibujar las celdas.
 */
class TileLibrary {
public:
  // Carga el PNG en 'path' y lo corta en tiles de tileSize x tileSize. Devuelve
  // false si la imagen no existe o no entra ningun tile completo.
  bool loadTileset(const QString &path, int tileSize);
  // Descarta el tileset cargado y vuelve al estado vacio.
  void clear();

  // Tile con id local 'id'; pixmap vacio si esta fuera de rango.
  const QPixmap &getTile(int id) const;
  // Spritesheet completo, sin trocear.
  const QPixmap &masterPixmap() const;

  int tileSize() const;              // lado del tile en px (0 si no hay tileset)
  int columns() const;               // tiles por fila
  int rows() const;                  // tiles por columna
  int tileCount() const;             // total de tiles (columns * rows)
  int usableWidth() const;           // ancho cubierto por tiles completos (px)
  int usableHeight() const;          // alto cubierto por tiles completos (px)
  const QString &sourcePath() const; // ruta del PNG cargado

private:
  QPixmap m_masterPixmap;       // spritesheet completo
  std::vector<QPixmap> m_tiles; // un pixmap por tile, indexado por id local
  QString m_sourcePath;
  int m_tileSize = 0;
  int m_columns = 0;
  int m_rows = 0;
};

#endif // EDITOR_RENDER_TILELIBRARY_H_
