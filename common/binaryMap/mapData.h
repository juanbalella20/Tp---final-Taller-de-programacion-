#ifndef MAP_DATA_H
#define MAP_DATA_H

#include <string>
#include <vector>

// Modelo de datos de un mapa, independiente de quien lo carga o guarda.
// Lo comparten el loader (.bin), el saver y el editor.

struct Tileset {
    std::string name;
    std::string file_path;  // path absoluto
    int columns;            // tiles por fila en el spritesheet
    int tile_count;         // total de tiles utiles
    int firstgid;           // primer ID global de este tileset
    bool collidable;        // si true, todos los tiles del set son collidables por defecto
};

// Definicion de un tile concreto
struct TileDef {
    int id;                  // ID global (incluye firstgid)
    int tileset_index;       // indice en tilesets
    int local_index;         // id - firstgid (posicion dentro del tileset)
    bool collidable;
};

// Una capa de mapa: matriz [row][col] de IDs globales. id=0 = celda vacia.
struct MapLayerData {
    std::string name;
    std::vector<std::vector<int>> data; // array de arrays
};

// Tile de teletransporte: su celda (x,y) en ESTA zona (collidable) y la zona
// destino (por nombre: "desert", "city", ...). El player aparece adyacente al
// teleport de la zona destino.
struct TeleportDef {
    int x;
    int y;
    std::string dest_zone;
};

#endif  // MAP_DATA_H
