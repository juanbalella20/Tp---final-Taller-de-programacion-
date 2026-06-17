#include "Map.h"

#include <stdexcept>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

Map::Map() {
  // Dos capas fijas, ambas WIDTH x HEIGHT llenas de 0 (celda vacia).
  layers_.resize(LAYER_COUNT);
  layers_[Ground].name = "ground";
  layers_[Buildings].name = "buildings";
  for (MapLayerData &layer : layers_) {
    layer.data.assign(HEIGHT, std::vector<int>(WIDTH, 0));
  }
  // Grilla de colision: todas las celdas transitables al crear el mapa.
  collision_.assign(HEIGHT, std::vector<uint8_t>(WIDTH, 0));
}

void Map::clear_canvas() {
  for (MapLayerData &layer : layers_) {
    layer.data.assign(HEIGHT, std::vector<int>(WIDTH, 0));
  }
  collision_.assign(HEIGHT, std::vector<uint8_t>(WIDTH, 0));
  teleports_.clear();
}

// --- Metadata (constantes) ---------------------------------------------------
int Map::width() const { return WIDTH; }
int Map::height() const { return HEIGHT; }
int Map::tile_size() const { return TILE_SIZE; }

bool Map::in_bounds(int x, int y) const {
  return x >= 0 && y >= 0 && x < WIDTH && y < HEIGHT;
}

// --- Tilesets ----------------------------------------------------------------
const std::vector<Tileset> &Map::tilesets() const { return tilesets_; }

std::vector<Tileset> Map::used_tilesets() const {
  std::unordered_set<int> used_indices;
  for (const MapLayerData &layer : layers_) {
    for (const std::vector<int> &row : layer.data) {
      for (int gid : row) {
        if (const TileDef *td = find_tile(gid))
          used_indices.insert(td->tileset_index);
      }
    }
  }
  std::vector<Tileset> result;
  for (int i = 0; i < static_cast<int>(tilesets_.size()); ++i) {
    if (used_indices.count(i))
      result.push_back(tilesets_[static_cast<std::size_t>(i)]);
  }
  return result;
}

int Map::add_tileset(const std::string &name, const std::string &file_path,
                     int columns, int tile_count, bool collidable) {
  if (columns <= 0 || tile_count <= 0) {
    throw std::runtime_error("Map::add_tileset: columns/tile_count invalidos");
  }
  Tileset ts;
  ts.name = name;
  ts.file_path = file_path;
  ts.columns = columns;
  ts.tile_count = tile_count;
  ts.firstgid = next_firstgid(); // asignacion automatica, sin huecos
  ts.collidable = collidable;
  tilesets_.push_back(std::move(ts));

  rebuild_tile_index();
  return static_cast<int>(tilesets_.size()) - 1;
}

void Map::clear_tilesets() {
  tilesets_.clear();
  rebuild_tile_index();
}

int Map::next_firstgid() const {
  // Arranca en 1 porque el gid 0 esta reservado para "celda vacia".
  int next = 1;
  for (const Tileset &ts : tilesets_) {
    int end = ts.firstgid + ts.tile_count;
    if (end > next)
      next = end;
  }
  return next;
}

const TileDef *Map::find_tile(int gid) const {
  // Misma logica que MapLoader::find_tile.
  if (gid == 0)
    return nullptr;
  auto it = tile_index_.find(gid);
  if (it == tile_index_.end())
    return nullptr;
  return &it->second;
}

// --- Capas (dos fijas: Ground=0, Buildings=1) --------------------------------
const std::vector<MapLayerData> &Map::layers() const { return layers_; }
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

// --- Colision (grilla por celda = unica fuente de verdad) --------------------
bool Map::is_collidable(int x, int y) const {
  if (x < 0 || y < 0 || x >= WIDTH || y >= HEIGHT)
    return true;
  return collision_[y][x] != 0;
}

const std::vector<std::vector<uint8_t>> &Map::collision() const {
  return collision_;
}

bool Map::is_blocked_cell(int x, int y) const {
  if (!in_bounds(x, y))
    return false;
  return collision_[y][x] != 0;
}

void Map::set_blocked(int x, int y, bool blocked) {
  if (!in_bounds(x, y))
    return;
  collision_[y][x] = blocked ? 1 : 0;
}

// --- Teleports ---------------------------------------------------------------
// Los teleports NO son una capa de tiles: se guardan como un vector de
// TeleportDef{x, y, dest_zone}. El borde amarillo del editor se dibuja a partir
// de este vector, y al guardar se vuelca a la seccion TELEPORTS del .bin.
const std::vector<TeleportDef> &Map::teleports() const { return teleports_; }

const TeleportDef *Map::teleport_at(int x, int y) const {
  for (const TeleportDef &tp : teleports_) {
    if (tp.x == x && tp.y == y)
      return &tp;
  }
  return nullptr;
}

void Map::add_teleport(int x, int y, const std::string &dest_zone) {
  if (!in_bounds(x, y))
    return;
  // Si ya existe un teleport en (x,y), actualiza su destino (no duplica).
  for (TeleportDef &tp : teleports_) {
    if (tp.x == x && tp.y == y) {
      tp.dest_zone = dest_zone;
      return;
    }
  }
  teleports_.push_back(TeleportDef{x, y, dest_zone});
}

void Map::remove_teleport(int x, int y) {
  for (auto it = teleports_.begin(); it != teleports_.end(); ++it) {
    if (it->x == x && it->y == y) {
      teleports_.erase(it);
      return;
    }
  }
}

void Map::load_from(std::vector<Tileset> tilesets,
                    std::vector<MapLayerData> layers,
                    std::vector<std::vector<uint8_t>> collision,
                    std::vector<TeleportDef> teleports) {
  tilesets_ = std::move(tilesets);
  layers_ = std::move(layers);
  collision_ = std::move(collision);
  teleports_ = std::move(teleports);
  rebuild_tile_index();
}

// --- Indice de tiles ---------------------------------------------------------
void Map::rebuild_tile_index() {
  // Identico a MapLoader::build_tile_index: por cada tileset y cada local en
  // [0, tile_count), registra tile_index_[firstgid+local].
  tile_index_.clear();
  for (int ts_idx = 0; ts_idx < static_cast<int>(tilesets_.size()); ++ts_idx) {
    const Tileset &ts = tilesets_[ts_idx];
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
