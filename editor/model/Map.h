#ifndef EDITOR_MODEL_MAP_H_
#define EDITOR_MODEL_MAP_H_

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

#include "constants/game_constants.h" // WIDTH, HEIGHT, TILE_SIZE
#include "mapLoader.h"                // Tileset, TileDef, MapLayerData

// Reutiliza los structs de common/mapLoader.h (Tileset, TileDef, MapLayerData)
class Map {
private:
  std::vector<Tileset> tilesets_;
  std::vector<MapLayerData> layers_;            // siempre LAYER_COUNT capas
  std::unordered_map<int, TileDef> tile_index_; // gid -> TileDef
  std::vector<TeleportDef> teleports_;          // celdas teleport + dest_zone
  std::vector<std::vector<uint8_t>> collision_; // [HEIGHT][WIDTH] (0/1)

  // Rearma tile_index_ a partir de tilesets_. = MapLoader::build_tile_index.
  void rebuild_tile_index();

public:
  // Indices de las capas fijas.
  enum LayerId { Ground = 0, Buildings = 1 };
  static constexpr int LAYER_COUNT = 2;

  // Mapa nuevo: WIDTH x HEIGHT, dos capas ("ground", "buildings") llenas de 0.
  Map();

  int width() const;     // ancho del mapa en tiles (constante WIDTH)
  int height() const;    // alto del mapa en tiles (constante HEIGHT)
  int tile_size() const; // lado del tile en px (constante TILE_SIZE)
  // true si (x,y) cae dentro del mapa.
  bool in_bounds(int x, int y) const;

  //  Tilesets
  const std::vector<Tileset> &tilesets() const;
  // Copia de los tilesets efectivamente referenciados por alguna capa (algun
  // gid dentro de su rango [firstgid, firstgid + tile_count)). Lo usa el
  // guardado para no serializar tilesets pre-cargados que el mapa no usa.
  std::vector<Tileset> used_tilesets() const;
  // Agrega un PNG como tileset. Asigna firstgid = next_firstgid().
  // Devuelve el indice del tileset agregado. Reconstruye el indice de tiles.
  int add_tileset(const std::string &name, const std::string &file_path,
                  int columns, int tile_count, bool collidable);
  // Descarta todos los tilesets (y su indice de tiles). Lo usa la pre-carga
  // para reemplazar la lista al abrir un mapa, en vez de appendear el catalogo
  // sobre los tilesets embebidos del .bin (lo que duplicaria gids).
  void clear_tilesets();
  // Mayor (firstgid + tile_count) entre los tilesets, o 1 si no hay ninguno.
  int next_firstgid() const;
  // Resuelve gid -> TileDef. nullptr si gid==0 o no registrado.
  const TileDef *find_tile(int gid) const;

  // Capas (Ground=0, Buildings=1)
  const std::vector<MapLayerData> &layers() const; // las LAYER_COUNT capas
  int layer_count() const;                         // LAYER_COUNT
  // gid de la celda (x,y) en 'layer'. Lanza out_of_range si esta fuera.
  int get_cell(int layer, int x, int y) const;
  // Escribe 'gid' en (x,y) de 'layer'. Lanza out_of_range si esta fuera.
  void set_cell(int layer, int x, int y, int gid);

  // true si la celda bloquea el paso (o cae fuera del mapa).
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
  // Marca (x,y) como teleport a 'dest_zone' (actualiza el destino si ya
  // existe).
  void add_teleport(int x, int y, const std::string &dest_zone);
  // Quita el teleport en (x,y) si lo hay.
  void remove_teleport(int x, int y);
};

#endif // EDITOR_MODEL_MAP_H_
