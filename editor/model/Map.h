#ifndef EDITOR_MAP_H
#define EDITOR_MAP_H

#include <string>
#include <unordered_map>
#include <vector>

#include "constants/game_constants.h"  // WIDTH, HEIGHT, TILE_SIZE
#include "mapLoader.h"                  // Tileset, TileDef, MapLayerData

/*
 * Modelo puro del mapa en edicion. SIN dependencias de Qt.
 *
 * Reutiliza los structs de common/mapLoader.h (Tileset, TileDef, MapLayerData)
 * para ser byte-compatible con el modelo del juego. A diferencia de MapLoader
 * (que solo sabe load(toml)), Map ofrece mutaciones celda-a-celda y carga de
 * tilesets, que es lo que el editor necesita.
 *
 * V1 SIMPLIFICADA:
 *  - Mapa de tamano FIJO WIDTH x HEIGHT (30x16), tile TILE_SIZE (64). No
 *    configurable.
 *  - DOS capas fijas: "ground" (indice 0, atras) y "buildings" (indice 1,
 *    adelante). No se agregan/quitan/renombran capas.
 *  - Sin spawns ni teleports (quedan para una version futura).
 *
 * Es el unico estado del mapa; lo muta exclusivamente el EditorDocument.
 *
 * Implementacion: Persona A (editor/model/Map.cpp). C++20.
 */
class Map {
public:
    // Indices de las dos capas fijas.
    enum LayerId { Ground = 0, Buildings = 1 };
    static constexpr int LAYER_COUNT = 2;

    // Mapa nuevo: WIDTH x HEIGHT, dos capas ("ground", "buildings") llenas de 0.
    Map();

    // --- Metadata (constantes) -------------------------------------------
    int width() const;       // WIDTH
    int height() const;      // HEIGHT
    int tile_size() const;   // TILE_SIZE
    bool in_bounds(int x, int y) const;

    // --- Tilesets ---------------------------------------------------------
    const std::vector<Tileset>& tilesets() const;
    // Agrega un PNG como tileset. Asigna firstgid = next_firstgid().
    // Devuelve el indice del tileset agregado. Reconstruye el indice de tiles.
    int add_tileset(const std::string& name, const std::string& file_path,
                    int columns, int tile_count, bool collidable);
    // Mayor (firstgid + tile_count) entre los tilesets, o 1 si no hay ninguno.
    int next_firstgid() const;
    // Resuelve gid -> TileDef. nullptr si gid==0 o no registrado.
    const TileDef* find_tile(int gid) const;

    // --- Capas (dos fijas: Ground=0, Buildings=1) -------------------------
    const std::vector<MapLayerData>& layers() const;
    int layer_count() const;                            // LAYER_COUNT
    int get_cell(int layer, int x, int y) const;        // gid; 0 si vacia
    void set_cell(int layer, int x, int y, int gid);    // mutacion de 1 celda

    // --- Colision (misma semantica que MapLoader::is_collidable) ----------
    bool is_collidable(int x, int y) const;

private:
    std::vector<Tileset> tilesets_;
    std::vector<MapLayerData> layers_;             // siempre LAYER_COUNT capas
    std::unordered_map<int, TileDef> tile_index_;  // gid -> TileDef

    // Rearma tile_index_ a partir de tilesets_. = MapLoader::build_tile_index.
    void rebuild_tile_index();
};

#endif  // EDITOR_MAP_H
