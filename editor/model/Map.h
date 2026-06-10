#ifndef EDITOR_MAP_H
#define EDITOR_MAP_H

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

#include "constants/game_constants.h" // WIDTH, HEIGHT, TILE_SIZE
#include "mapLoader.h"                // Tileset, TileDef, MapLayerData

// Reutiliza los structs de common/mapLoader.h (Tileset, TileDef, MapLayerData)
class Map {
public:
  // Indices de las capas fijas.
  enum LayerId { Ground = 0, Buildings = 1 };
  static constexpr int LAYER_COUNT = 2;

  // Mapa nuevo: WIDTH x HEIGHT, dos capas ("ground", "buildings") llenas de 0.
  Map();

  int width() const;     // WIDTH
  int height() const;    // HEIGHT
  int tile_size() const; // TILE_SIZE
  bool in_bounds(int x, int y) const;

  //  Tilesets
  const std::vector<Tileset> &tilesets() const;
  // Agrega un PNG como tileset. Asigna firstgid = next_firstgid().
  // Devuelve el indice del tileset agregado. Reconstruye el indice de tiles.
  int add_tileset(const std::string &name, const std::string &file_path,
                  int columns, int tile_count, bool collidable);
  // Mayor (firstgid + tile_count) entre los tilesets, o 1 si no hay ninguno.
  int next_firstgid() const;
  // Resuelve gid -> TileDef. nullptr si gid==0 o no registrado.
  const TileDef *find_tile(int gid) const;

  // Capas (Ground=0, Buildings=1)
  const std::vector<MapLayerData> &layers() const;
  int layer_count() const; // LAYER_COUNT
  int get_cell(int layer, int x, int y) const;
  void set_cell(int layer, int x, int y, int gid);

  bool is_collidable(int x, int y) const;

  // Reemplaza todo el estado del mapa (lo usa EditorDocument::open al cargar un
  // .bin). Reconstruye el indice de tiles a partir de los tilesets dados.
  void load_from(std::vector<Tileset> tilesets,
                 std::vector<MapLayerData> layers,
                 std::vector<std::vector<uint8_t>> collision,
                 std::vector<TeleportDef> teleports);

  // Colision: grilla por celda, fuente unica de verdad (independiente de los
  // graficos). La pinta la herramienta Colision y se vuelca a la seccion
  // COLLISION del .bin.
  const std::vector<std::vector<uint8_t>> &collision() const;
  bool is_blocked_cell(int x, int y) const;     // false si fuera de rango
  void set_blocked(int x, int y, bool blocked); // no-op si fuera de rango

  // Teleports
  const std::vector<TeleportDef> &teleports() const;
  const TeleportDef *teleport_at(int x, int y) const; // nullptr si no hay
  void add_teleport(int x, int y, const std::string &dest_zone);
  void remove_teleport(int x, int y);

private:
  std::vector<Tileset> tilesets_;
  std::vector<MapLayerData> layers_;            // siempre LAYER_COUNT capas
  std::unordered_map<int, TileDef> tile_index_; // gid -> TileDef
  std::vector<TeleportDef> teleports_;          // celdas teleport + dest_zone
  std::vector<std::vector<uint8_t>> collision_; // [HEIGHT][WIDTH] (0/1)

  // Rearma tile_index_ a partir de tilesets_. = MapLoader::build_tile_index.
  void rebuild_tile_index();
};

#endif // EDITOR_MAP_H
