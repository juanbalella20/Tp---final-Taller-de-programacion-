#ifndef GAME_MAP_H_
#define GAME_MAP_H_

#include "../../common/commands/clientCmd.h"
#include "player/player.h"
#include "npc/npcHostile.h"
#include "../../common/constants/game_constants.h"
#include "../../common/constants/protocol_constants.h"
#include "../../common/info/item_info.h"
#include "../../common/info/npc_info.h"
#include "../../common/info/item_floor_info.h"
#include "../../common/mapLoader.h"
#include "npc/npcSeller.h"
#include "item/item.h"
#include "zoneWorld.h"

#include <vector>
#include <map>
#include <string>
#include <utility>
#include "playerinfo.h"

struct NpcSpawn {
    std::string type;
    int x;
    int y;
};

struct ItemSpawn {
    std::string type;
    int x;
    int y;
};

struct InitialState {
    std::vector<NpcSpawn> npcs;
    std::vector<ItemSpawn> items;
};

struct TeleportResult {
        bool adyacente;
        Zone dest_zone;
        int x;
        int y;
};

// Factory: construye un NPChostile a partir de un NpcSpawn
NPChostile make_npc_from_spawn(const NpcSpawn& spawn);


class GameMap {

private:
    std::map<Zone, ZoneWorld> zones;          // enum Zone -> mundo de esa zona
    std::vector<Player> players;              // players globales (atraviesan zonas)
    std::map<std::string, Zone> player_zone;  // player_name -> zona actual (tag)

    Player* find_player_by_name(const std::string& name);

    // Helpers de orquestacion: resuelven la zona de un player
    ZoneWorld& zone_of(const std::string& player_name);
    Zone zone_id_of(const std::string& player_name) const;
    // Punteros (const) a los players que estan en la zona z
    std::vector<const Player*> players_in(Zone z) const;

    // Celda donde aparece un player que llega a la zona dst: adyacente al
    // teleport de esa zona si tiene, o una celda libre random. {-1,-1} si nada.
    std::pair<int, int> find_arrival_cell(ZoneWorld& dst, Zone dest_zone);

public:
    GameMap();

    // Mapa (matriz de terreno) de la zona donde está el player
    std::vector<std::vector<elements>> get_map(const std::string& player_name);

    void add_player(Player player);

    // Devuelve un snapshot de todos los jugadores en el mismo mapa que player_name
    // (excluyéndolo a él). Hoy todo está en "desert"; cuando haya múltiples mapas,
    // filtrar por sector aquí.
    std::vector<PlayerInfo> build_players_snapshot(const std::string& player_name);
    struct MoveResult {
        bool moved;
        std::string player_name;
        int new_x;
        int new_y;
    };

    struct AttackResult {
        bool hit;
        bool entity_died;
        std::string entity_name;
    };

    // Calcula la nueva posicion del player a partir de su posicion actual y la
    // direccion, resolviendo limites/terreno/actores contra SU zona
    MoveResult try_move(Direction dir, const std::string& player_name);

    // Ataca la celda (x,y) en la zona del atacante
    AttackResult attack(const std::string& atacker_name, int x, int y);

    // Respawn de NPCs en TODAS las zonas. Devuelve true si hubo alguno
    bool update_npcs();

    // Snapshots de la zona del player indicado
    std::vector<NpcInfo> build_npcs_snapshot(const std::string& player_name);
    std::vector<ItemFloorInfo> build_items_snapshot(const std::string& player_name);

    // Comercio: el seller se ubica por (x,y) en la zona del player
    bool player_sell_item(const std::string& player_name, int x, int y, const std::string& item_id);
    bool player_buy_item(const std::string& player_name, int x, int y, const std::string& item_id);
    std::vector<ItemInfo> list_seller_items(const std::string& player_name, int x, int y);
    void player_deposit_item(const std::string& player_name, const std::string& item_id);
    void player_deposit_gold(const std::string& player_name, int amount);
    void player_retire_item(const std::string& player_name, const std::string& item_id);
    void player_retire_gold(const std::string& player_name, int amount);

    // Zona actual del player (para MSG_ZONE_CHANGE)
    Zone get_player_zone(const std::string& player_name) const;


    // Si el player esta adyacente a un teleport, lo mueve a la zona destino y actualiza su tag.
    TeleportResult teleport_player(const std::string& player_name);

    void player_equip_item(const std::string& player_name, const std::string& item_id);
    void spawn_player(const std::string& name);
    const Player& get_player(const std::string& name);
    bool player_exists(const std::string& name);

    std::unique_ptr<Item> pick_up_item(const std::string& player_name);
    void give_item_to_player(const std::string& player_name, std::unique_ptr<Item> item);

    uint32_t get_player_gold(const std::string& name);
    uint32_t get_player_hp(const std::string& name);
    uint32_t get_player_xp(const std::string& name);
    uint32_t get_player_mana(const std::string& name);

    /*
     * Carga TODAS las zonas al iniciar el server. Por cada (zone_id, path):
     * crea la ZoneWorld, carga su terreno y spawnea sus actores segun el
     * InitialState correspondiente.
     */
    void init_world(const std::map<Zone, std::string>& zone_paths,
                    const std::map<Zone, InitialState>& initial_states);

    bool pick_up_gold(const std::string& player_name);
};

#endif
