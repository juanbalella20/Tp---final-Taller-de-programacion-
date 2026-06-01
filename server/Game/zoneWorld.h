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

class Player;
class Item;

// Item tirado en el piso de una zona
struct groundItem {
    positionCoord pos;
    std::unique_ptr<Item> item;
};


class ZoneWorld {
private:
    int width = 0;
    int height = 0;
    std::vector<std::vector<elements>> map;
    std::map<std::string, positionCoord> spawns;
    std::vector<NPChostile> npcs;
    std::vector<NPCseller> sellers;
    std::vector<NPCbanker> bankers;
    std::vector<groundItem> ground_items;
    std::vector<groundGold> ground_gold;
    std::vector<TeleportDef> teleports;

public:
    ZoneWorld() = default;

    // No copiable (ground_items tiene unique_ptr); movible
    ZoneWorld(const ZoneWorld&) = delete;
    ZoneWorld& operator=(const ZoneWorld&) = delete;
    ZoneWorld(ZoneWorld&&) = default;
    ZoneWorld& operator=(ZoneWorld&&) = default;

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
    bool has_actor_at(int x, int y, const std::vector<const Player*>& players_here) const;
    NPCseller* seller_adjacent_to(int px, int py);
    // Hay un item en el piso en (x,y)?
    bool has_ground_item_at(int x, int y) const;
    std::pair<int, int> find_random_empty_cell(
        const std::vector<const Player*>& players_here) const;

    // --- Spawns / mutaciones de esta zona ---
    void spawn_npc(NPChostile&& npc);
    void spawn_seller(int x, int y);
    void spawn_banker(int x, int y);
    void spawn_item(int x, int y, std::unique_ptr<Item> item);
    void spawn_gold(int x, int y, int amount);

    // Respawn de NPCs muertos cuyo timer venció. Devuelve true si hubo alguno.
    bool update_npcs();

    // --- Snapshots de contenido para mandar al cliente ---
    std::vector<NpcInfo> build_npcs_snapshot() const;
    std::vector<ItemFloorInfo> build_items_snapshot() const;

    // --- Sellers ---
    // Devuelve el seller en (x,y) o nullptr si no hay.
    NPCseller* seller_at(int x, int y);
    std::vector<ItemInfo> list_seller_items(int x, int y);

    // --- Items / oro en el piso ---
    // Saca y devuelve un item adyacente a (px,py), o nullptr.
    std::unique_ptr<Item> take_item_near(int px, int py);
    // Saca el oro que esté exactamente en (px,py); devuelve el monto o 0.
    int take_gold_at(int px, int py);

    // Busca un NPC hostile vivo en (x,y); nullptr si no hay.
    NPChostile* hostile_at(int x, int y);

    // --- Teleports ---
    const std::vector<TeleportDef>& get_teleports() const { return teleports; }
    // Teleport cuya celda sea adyacente (Manhattan <= 1) a (x,y), o nullptr.
    const TeleportDef* teleport_adjacent_to(int x, int y) const;
    // Celda libre (terreno empty, sin actor) adyacente a (tx,ty); si no hay
    // adyacente libre, cae a find_random_empty_cell. Devuelve {-1,-1} si nada.
    std::pair<int, int> free_cell_adjacent_to(
        int tx, int ty, const std::vector<const Player*>& players_here) const;
};

#endif
