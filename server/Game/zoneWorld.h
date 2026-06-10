#ifndef ZONE_WORLD_H_
#define ZONE_WORLD_H_

#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "../../common/constants/game_constants.h"
#include "../../common/info/item_info.h"
#include "../../common/info/item_floor_info.h"
#include "../../common/info/npc_info.h"
#include "../../common/mapLoader.h"
#include "npc/npcHostile.h"
#include "npc/npcSeller.h"
#include "npc/npcBanker.h"
#include "npc/npcPriest.h"


class Player;
class Item;

// Item tirado en el piso de una zona
struct groundItem {
    positionCoord pos;
    std::unique_ptr<Item> item;
};

struct NPCAttackEvent {
    std::string victim_name;
    std::string npc_name;
    int damage;
    bool dodged;
    bool victim_died;
};

// Punto de spawn de un NPC hostil (tipo + posicion). Lo consume el factory
// make_npc_from_spawn.
struct NpcSpawn {
    std::string type;
    int x;
    int y;
};

// Receta de poblado de una zona: qué cargar (terreno) y con qué llenarla.
// La arma quien conoce GameConfig (GameLoop), de modo que ZoneWorld no depende
// del singleton: recibe los datos ya resueltos.
struct ZoneSpawnConfig {
    std::string terrain_path;
    int num_npc = 0;
    int num_items = 0;
    // Tipos de NPC hostil permitidos en la zona (strings que reconoce
    // make_npc_from_spawn). Vacío => no se spawnean hostiles.
    std::vector<std::string> npc_types;
    // Ids de items que pueden caer en la zona (los reconoce ItemCatalog). Los
    // num_items se sortean de esta lista. Vacío => pool global por defecto.
    std::vector<std::string> item_types;
    // Cantidad de NPCs amigos a spawnear en celdas random (además de los que
    // vengan como [[spawn]] nombrados en el .bin). 0 = ninguno.
    int num_priests = 0;
    int num_sellers = 0;
    int num_bankers = 0;
};

// Factory: construye un NPChostile a partir de un NpcSpawn, resolviendo sus
// stats desde GameConfig segun el tipo.
NPChostile make_npc_from_spawn(const NpcSpawn& spawn);


class ZoneWorld {
private:
    int width = 0;
    int height = 0;
    std::vector<std::vector<elements>> map;
    std::map<std::string, positionCoord> spawns;
    std::vector<NPChostile> npcs;
    std::vector<NPCseller> sellers;
    std::vector<NPCbanker> bankers;
    std::vector<NPCpriest> priests;
    std::vector<groundItem> ground_items;
    std::vector<groundGold> ground_gold;
    std::vector<TeleportDef> teleports;

    // persecución de npcs hacia players
    static constexpr int AGGRO_RANGE = 8;
    static constexpr int ABANDON_RANGE = 15;

    // Pobla la zona segun la receta: NPCs hostiles random (de cfg.npc_types),
    // luego items random, luego NPCs amigos. Se llama desde init() con el
    // terreno ya cargado y sin players en la zona.
    void spawn(const ZoneSpawnConfig& cfg);

    // Elige un tipo permitido al azar y construye un NPChostile en una celda
    // libre. Si no hay tipos permitidos, devuelve un NPC en {-1,-1} (descartado
    // por spawn_npc).
    NPChostile rand_hostile(const std::vector<std::string>& npc_types);

public:
    ZoneWorld() = default;

    // No copiable (ground_items tiene unique_ptr); movible
    ZoneWorld(const ZoneWorld&) = delete;
    ZoneWorld& operator=(const ZoneWorld&) = delete;
    ZoneWorld(ZoneWorld&&) = default;
    ZoneWorld& operator=(ZoneWorld&&) = default;

    // Punto de entrada único de inicialización: carga el terreno desde
    // cfg.terrain_path y puebla la zona (NPCs hostiles, items y NPCs amigos)
    // segun cfg. Tras esta llamada la zona queda lista para usarse.
    void init(const ZoneSpawnConfig& cfg);

    // Carga el terreno desde un TOML: setea width/height, arma la matriz con
    // los tiles colidables como elements::buildings y copia los spawns.
    void load_terrain(const std::string& toml_path);

    int get_width() const { return width; }
    int get_height() const { return height; }
    const std::vector<std::vector<elements>>& get_map() const { return map; }
    const std::map<std::string, positionCoord>& get_spawns() const { return spawns; }

    // --- Consultas de celda ---
    bool in_bounds(int x, int y) const;
    bool is_blocked_terrain(int x, int y) const;  // map[y][x] != empty
    // Hay un actor (npc vivo o alguno de los players_here) en (x,y)?
    bool has_actor_at(int x, int y, const std::vector<Player*>& players_here) const;
    // Hay un item en el piso en (x,y)?
    bool has_ground_item_at(int x, int y) const;
    std::pair<int, int> find_random_empty_cell(
        const std::vector<Player*>& players_here) const;

    // --- Spawns / mutaciones de esta zona ---
    void spawn_npc(NPChostile&& npc);
    void spawn_seller(int x, int y);
    void spawn_banker(int x, int y);
    void spawn_priest(int x, int y);
    void spawn_item(int x, int y, std::unique_ptr<Item> item);
    void spawn_gold(int x, int y, int amount);

    // Esparce los items alrededor de (center_x, center_y) en anillos crecientes
    // (8 vecinas primero, luego más lejos). Saltea celdas bloqueadas, con actor
    // o con un item ya tirado. Cada item ocupa una celda libre distinta.
    void scatter_items(int center_x, int center_y,
                       std::vector<std::unique_ptr<Item>> items,
                       const std::vector<Player*>& players_here);

    // Respawn de NPCs muertos cuyo timer venció. Devuelve true si hubo alguno.
    bool update_npcs();

    std::vector<NPCAttackEvent> update_npc_aggro(const std::vector<Player*>& players_in_zone);

    // --- Snapshots de contenido para mandar al cliente ---
    std::vector<NpcInfo> build_npcs_snapshot() const;
    std::vector<ItemFloorInfo> build_items_snapshot() const;

    // --- Sellers ---
    // Devuelve el seller en (x,y) o nullptr si no hay.
    NPCseller* seller_at(int x, int y);
    std::vector<ItemInfo> list_seller_items(int x, int y);
    NPCseller* seller_adjacent_to(int px, int py);
    //--- Bankers---
    NPCbanker* banker_adjacent_to(int px, int py);
    std::string get_adjacent_friendly_type(int px, int py);

    // --- Items / oro en el piso ---
    // Saca y devuelve un item adyacente a (px,py), o nullptr.
    std::unique_ptr<Item> take_item_near(int px, int py);
    // Saca el oro que esté exactamente en (px,py); devuelve el monto o 0.
    int take_gold_at(int px, int py);

    // Busca un NPC hostile vivo en (x,y); nullptr si no hay.
    NPChostile* hostile_at(int x, int y);

    // --- Teleports ---
    const std::vector<TeleportDef>& get_teleports() const { return teleports; }
    // Celda libre (terreno empty, sin actor) adyacente a (tx,ty); si no hay
    // adyacente libre, cae a find_random_empty_cell. Devuelve {-1,-1} si nada.
    std::pair<int, int> free_cell_adjacent_to(
        int tx, int ty, const std::vector<Player*>& players_here) const;
    // Recorre teleports y devuelve el que coincide con (x,y)
    const TeleportDef* teleport_at(int x, int y) const;
};

#endif
