#include "TilesetPixmapCache.h"

#include <QString>

#include "Map.h"
#include "game_constants.h"  // TILE_SIZE

void TilesetPixmapCache::sync(const Map& map) {
    const auto& tilesets = map.tilesets();
    // Carga los pixmaps de los tilesets que aun no esten cacheados (los nuevos
    // siempre se agregan al final, asi que basta cargar desde pixmaps_.size()).
    for (size_t i = pixmaps_.size(); i < tilesets.size(); ++i) {
        QPixmap pix(QString::fromStdString(tilesets[i].file_path));
        pixmaps_.push_back(pix);  // si falla la carga, pix queda null (fallback)
    }
}

const QPixmap* TilesetPixmapCache::pixmap_for_tileset(int index) const {
    if (index < 0 || index >= static_cast<int>(pixmaps_.size())) return nullptr;
    return &pixmaps_[index];
}

QRect TilesetPixmapCache::src_rect(const Map& map, int gid) const {
    const TileDef* td = map.find_tile(gid);
    if (!td) return QRect();

    const auto& tilesets = map.tilesets();
    if (td->tileset_index < 0 ||
        td->tileset_index >= static_cast<int>(tilesets.size())) {
        return QRect();
    }
    int columns = tilesets[td->tileset_index].columns;
    int col = td->local_index % columns;
    int row = td->local_index / columns;
    return QRect(col * TILE_SIZE, row * TILE_SIZE, TILE_SIZE, TILE_SIZE);
}

void TilesetPixmapCache::clear() { pixmaps_.clear(); }
