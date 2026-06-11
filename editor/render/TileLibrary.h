#ifndef EDITOR_RENDER_TILELIBRARY_H_
#define EDITOR_RENDER_TILELIBRARY_H_

#include <QPixmap>
#include <QString>

#include <vector>

class TileLibrary {
public:
  bool loadTileset(const QString &path, int tileSize);
  void clear();

  const QPixmap &getTile(int id) const;
  const QPixmap &masterPixmap() const;

  int tileSize() const;
  int columns() const;
  int rows() const;
  int tileCount() const;
  int usableWidth() const;
  int usableHeight() const;
  const QString &sourcePath() const;

private:
  // mapa entero de tiles
  QPixmap m_masterPixmap;
  // cada tile por separado
  std::vector<QPixmap> m_tiles;
  QString m_sourcePath;
  int m_tileSize = 0;
  int m_columns = 0;
  int m_rows = 0;
};

#endif // EDITOR_RENDER_TILELIBRARY_H_
