#ifndef BINARY_MAP_SAVER_H
#define BINARY_MAP_SAVER_H

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
 * V1: escribe META + TILESETS + LAYERS. Spawns/teleports quedan fuera (sus
 * secciones estan reservadas en binaryMapFormat.h para una version futura).
 *
 * Implementacion: Persona A (common/binaryMapSaver.cpp). C++17.
 */
class BinaryMapSaver {
public:
    /*
     * Serializa el mapa a 'path' en big-endian usando network byte order.
     * Escribe las secciones META, TILESETS, LAYERS (section_count = 3).
     *
     * Precondiciones (responsabilidad del llamador): cada layer.data es una
     * matriz [height][width]; los gid caben en uint16.
     *
     * Lanza std::runtime_error ante error de I/O.
     */
    static void save(const std::string& path,
                     int tile_size, int width, int height,
                     const std::vector<Tileset>& tilesets,
                     const std::vector<MapLayerData>& layers);
};

#endif  // BINARY_MAP_SAVER_H
