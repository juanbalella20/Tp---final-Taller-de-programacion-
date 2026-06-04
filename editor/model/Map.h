#ifndef EDITOR_MAP_H
#define EDITOR_MAP_H

#include <string>
#include <unordered_map>
#include <vector>

#include "constants/game_constants.h"  // WIDTH, HEIGHT, TILE_SIZE
#include "mapLoader.h"                  // Tileset, TileDef, MapLayerData

// Reutiliza los structs de common/mapLoader.h (Tileset, TileDef, MapLayerData)
class Map {
public:
    // Indices de las tres capas fijas.
    enum LayerId { Ground = 0, Buildings = 1, Teleports = 2 };
    static constexpr int LAYER_COUNT = 3;

    // gid marcador en la capa Teleports: 1 = celda apta para teleport, 0 = no.
    // No pertenece a ningun tileset, asi que no se dibuja como tile ni colisiona.
    static constexpr int TELEPORT_MARKER = 1;

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
