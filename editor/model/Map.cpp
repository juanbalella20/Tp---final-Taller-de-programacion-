#include "Map.h"

#include <stdexcept>

// ============================================================================
// Implementacion del modelo puro del mapa (Persona A). SIN Qt.
//
// V1: tamano FIJO WIDTH x HEIGHT (30x16), tile TILE_SIZE (64), DOS capas fijas
// ("ground" indice 0, "buildings" indice 1). Sin spawns ni teleports.
//
// Reutiliza la semantica de MapLoader para tilesets/colision: build_tile_index,
// find_tile e is_collidable son identicos para que el mundo que ve el editor
// coincida con el que ve el server.
// ============================================================================

Map::Map() {
    // Dos capas fijas, ambas WIDTH x HEIGHT llenas de 0 (celda vacia).
    layers_.resize(LAYER_COUNT);
    layers_[Ground].name    = "ground";
    layers_[Buildings].name = "buildings";
    for (MapLayerData& layer : layers_) {
        layer.data.assign(HEIGHT, std::vector<int>(WIDTH, 0));
    }
}

// --- Metadata (constantes) ---------------------------------------------------
int Map::width() const { return WIDTH; }
int Map::height() const { return HEIGHT; }
int Map::tile_size() const { return TILE_SIZE; }

bool Map::in_bounds(int x, int y) const {
    return x >= 0 && y >= 0 && x < WIDTH && y < HEIGHT;
}

// --- Tilesets ----------------------------------------------------------------
const std::vector<Tileset>& Map::tilesets() const { return tilesets_; }

int Map::add_tileset(const std::string& name, const std::string& file_path,
                     int columns, int tile_count, bool collidable) {
    if (columns <= 0 || tile_count <= 0) {
        throw std::runtime_error("Map::add_tileset: columns/tile_count invalidos");
    }
    Tileset ts;
    ts.name       = name;
    ts.file_path  = file_path;
    ts.columns    = columns;
    ts.tile_count = tile_count;
    ts.firstgid   = next_firstgid();  // asignacion automatica, sin huecos
    ts.collidable = collidable;
    tilesets_.push_back(std::move(ts));

    rebuild_tile_index();
    return static_cast<int>(tilesets_.size()) - 1;
}

int Map::next_firstgid() const {
    // Mayor (firstgid + tile_count) entre los tilesets, o 1 si no hay ninguno.
    // Arranca en 1 porque el gid 0 esta reservado para "celda vacia".
    int next = 1;
    for (const Tileset& ts : tilesets_) {
        int end = ts.firstgid + ts.tile_count;
        if (end > next) next = end;
    }
    return next;
}

const TileDef* Map::find_tile(int gid) const {
    // Misma logica que MapLoader::find_tile.
    if (gid == 0) return nullptr;
    auto it = tile_index_.find(gid);
    if (it == tile_index_.end()) return nullptr;
    return &it->second;
}

// --- Capas (dos fijas: Ground=0, Buildings=1) --------------------------------
const std::vector<MapLayerData>& Map::layers() const { return layers_; }
int Map::layer_count() const { return LAYER_COUNT; }

int Map::get_cell(int layer, int x, int y) const {
    if (layer < 0 || layer >= LAYER_COUNT || !in_bounds(x, y)) {
        throw std::out_of_range("Map::get_cell: indice fuera de rango");
    }
    return layers_[layer].data[y][x];
}

void Map::set_cell(int layer, int x, int y, int gid) {
    if (layer < 0 || layer >= LAYER_COUNT || !in_bounds(x, y)) {
        throw std::out_of_range("Map::set_cell: indice fuera de rango");
    }
    layers_[layer].data[y][x] = gid;
}

// --- Colision (misma semantica que MapLoader::is_collidable) -----------------
bool Map::is_collidable(int x, int y) const {
    if (x < 0 || y < 0 || x >= WIDTH || y >= HEIGHT) return true;
    for (const auto& layer : layers_) {
        if (y >= static_cast<int>(layer.data.size())) continue;
        const auto& row = layer.data[y];
        if (x >= static_cast<int>(row.size())) continue;
        int id = row[x];
        const TileDef* td = find_tile(id);
        if (td && td->collidable) return true;
    }
    return false;
}

// --- Indice de tiles ---------------------------------------------------------
void Map::rebuild_tile_index() {
    // Identico a MapLoader::build_tile_index: por cada tileset y cada local en
    // [0, tile_count), registra tile_index_[firstgid+local].
    tile_index_.clear();
    for (int ts_idx = 0; ts_idx < static_cast<int>(tilesets_.size()); ++ts_idx) {
        const Tileset& ts = tilesets_[ts_idx];
        for (int local = 0; local < ts.tile_count; ++local) {
            TileDef td;
            td.id = ts.firstgid + local;
            td.tileset_index = ts_idx;
            td.local_index = local;
            td.collidable = ts.collidable;
            tile_index_[td.id] = td;
        }
    }
}
