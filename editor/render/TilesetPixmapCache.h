#ifndef TILESET_PIXMAP_CACHE_H
#define TILESET_PIXMAP_CACHE_H

#include <QPixmap>
#include <QRect>
#include <vector>

class Map;


class TilesetPixmapCache {
public:
    // Carga (o recarga) los QPixmap de los tilesets del Map cuyo indice aun no
    // este cacheado. Llamar tras agregar tilesets o tras abrir un mapa.
    void sync(const Map& map);

    // QPixmap del tileset 'index', o nullptr si no esta cacheado.
    const QPixmap* pixmap_for_tileset(int index) const;

    // Sub-rect dentro del spritesheet para 'gid'. QRect vacio si gid invalido.
    QRect src_rect(const Map& map, int gid) const;

    void clear();

private:
    std::vector<QPixmap> pixmaps_;  // alineado con Map::tilesets()
};

#endif  // TILESET_PIXMAP_CACHE_H
