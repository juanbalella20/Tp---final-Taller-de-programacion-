#ifndef BINARY_MAP_SAVER_H
#define BINARY_MAP_SAVER_H

#include <cstdint>
#include <string>
#include <vector>

#include "mapData.h"  // Tileset, MapLayerData, TeleportDef

class BinaryMapSaver {
public:
    /*
     * Serializa el mapa a 'path' en big-endian usando network byte order.
     * Escribe las secciones META, TILESETS, LAYERS, TELEPORTS (section_count = 4).
     */
    static void save(const std::string& path,
                     int tile_size, int width, int height,
                     const std::vector<Tileset>& tilesets,
                     const std::vector<MapLayerData>& layers,
                     const std::vector<TeleportDef>& teleports,
                     const std::vector<std::vector<uint8_t>>& collision);
};

#endif  // BINARY_MAP_SAVER_H
