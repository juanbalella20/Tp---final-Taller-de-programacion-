#include "TileLibrary.h"

#include <utility>

bool TileLibrary::loadTileset(const QString &path, int tileSize) {
  clear();
  if (tileSize <= 0)
    return false;

  QPixmap master(path);
  if (master.isNull())
    return false;

  // Calcula cuántos tiles completos entran horizontal y
  // verticalmente
  // Si imagen es de 128x128 y tiles de 64x64
  // cols = 128 / 64 = 2
  // rows = 128 / 64 = 2
  // tiles 2*2 = 4
  const int cols = master.width() / tileSize;
  const int r = master.height() / tileSize;
  if (cols <= 0 || r <= 0)
    return false;

  
  m_tiles.reserve(static_cast<std::size_t>(cols * r));
  for (int row = 0; row < r; ++row) {
    for (int column = 0; column < cols; ++column) {
      m_tiles.push_back(
          master.copy(column * tileSize, row * tileSize, tileSize, tileSize));
    }
  }

  m_masterPixmap = std::move(master);
  m_sourcePath = path;
  m_tileSize = tileSize;
  m_columns = cols;
  m_rows = r;
  return true;
}

void TileLibrary::clear() {
  m_masterPixmap = QPixmap();
  m_tiles.clear();
  m_sourcePath.clear();
  m_tileSize = 0;
  m_columns = 0;
  m_rows = 0;
}

const QPixmap &TileLibrary::getTile(int id) const {
  static const QPixmap emptyPixmap;
  if (id < 0 || id >= static_cast<int>(m_tiles.size()))
    return emptyPixmap;
  return m_tiles[static_cast<std::size_t>(id)];
}

const QPixmap &TileLibrary::masterPixmap() const { return m_masterPixmap; }

int TileLibrary::tileSize() const { return m_tileSize; }
int TileLibrary::columns() const { return m_columns; }
int TileLibrary::rows() const { return m_rows; }
int TileLibrary::tileCount() const { return static_cast<int>(m_tiles.size()); }
int TileLibrary::usableWidth() const { return m_columns * m_tileSize; }
int TileLibrary::usableHeight() const { return m_rows * m_tileSize; }
const QString &TileLibrary::sourcePath() const { return m_sourcePath; }
