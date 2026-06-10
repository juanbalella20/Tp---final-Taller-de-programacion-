#ifndef EDITOR_TILE_LIBRARY_H
#define EDITOR_TILE_LIBRARY_H

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
  QPixmap m_masterPixmap;
  std::vector<QPixmap> m_tiles;
  QString m_sourcePath;
  int m_tileSize = 0;
  int m_columns = 0;
  int m_rows = 0;
};

#endif // EDITOR_TILE_LIBRARY_H
