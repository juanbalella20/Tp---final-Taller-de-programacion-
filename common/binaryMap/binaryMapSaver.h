#ifndef BINARY_MAP_SAVER_H
#define BINARY_MAP_SAVER_H

#include <cstdint>
#include <string>
#include <vector>

#include "mapLoader.h"  // Tileset, MapLayerData

/*
 * Escribe un mapa al formato binario .bin (ver binaryMapFormat.h).
 *
 * NO depende del Map del editor: recibe los datos como vectores sueltos. Asi
 * esta clase vive en common/ y puede ser usada tanto por el editor (que extrae
 * estos datos de su Map) como por cualquier otro productor. La persistencia no
 * conoce al editor.
 *
 * Escribe META + TILESETS + LAYERS + TELEPORTS + COLLISION. Spawns quedan fuera
 * (su seccion esta reservada en binaryMapFormat.h para una version futura).
 *
 * Implementacion: Persona A (common/binaryMapSaver.cpp). C++17.
 */
class BinaryMapSaver {
public:
    /*
     * Serializa el mapa a 'path' en big-endian usando network byte order.
     * Escribe las secciones META, TILESETS, LAYERS, TELEPORTS (section_count = 4).
     *
     * Precondiciones (responsabilidad del llamador): cada layer.data es una
     * matriz [height][width]; los gid caben en uint16.
     *
     * Lanza std::runtime_error ante error de I/O.
     */
    static void save(const std::string& path,
                     int tile_size, int width, int height,
                     const std::vector<Tileset>& tilesets,
                     const std::vector<MapLayerData>& layers,
                     const std::vector<TeleportDef>& teleports,
                     const std::vector<std::vector<uint8_t>>& collision);
};

#endif  // BINARY_MAP_SAVER_H
