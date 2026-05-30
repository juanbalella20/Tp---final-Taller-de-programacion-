#ifndef GAME_MAP_H_
#define GAME_MAP_H_

#include "../../common/commands/clientCmd.h"
#include "player/player.h"
#include "npc/npcHostile.h"
#include "../../common/constants/game_constants.h"
#include "../../common/info/item_info.h"
#include "../../common/info/npc_info.h"
#include "../../common/info/item_floor_info.h"
#include "../../common/mapLoader.h"
#include "npc/npcSeller.h"
#include "item/item.h"

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

struct groundItem {
    positionCoord pos;
    std::unique_ptr<Item> item;
};

struct InitialState {
    std::vector<NpcSpawn> npcs;
    std::vector<ItemSpawn> items;
};

// Factory: construye un NPChostile a partir de un NpcSpawn
NPChostile make_npc_from_spawn(const NpcSpawn& spawn);

class GameMap {

private:
    int width = 0;
    int height = 0;
    std::vector<std::vector<elements>> map;
    std::vector<Player> players;
    std::vector<NPChostile> npcs;
    std::vector<ItemInfo> items;
    std::map<std::string, positionCoord> spawns;
    std::map<std::string, positionCoord> player_position;
    //TileGrid tiles;
    std::vector<groundItem> ground_items;
    std::vector<groundGold> ground_gold;
    std::vector<NPCseller> sellers;

    Player* find_player_by_name(const std::string& name);
    bool look_for_entity(int x, int y);
    Entity* find_entity_at(int x, int y);
    std::pair<int,int> find_random_empty_cell();
    bool has_actor_at(int x, int y);

public:
    GameMap();

    std::vector<std::vector<elements>> get_map();

    int get_width()  const { return width; }
    int get_height() const { return height; }

    const std::map<std::string, positionCoord>& get_spawns() const { return spawns; }

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

    // Calcula la nueva posicion del player a partir de su posicion actual
    // y la direccion. Si es valida, la aplica y devuelve {true, name, x, y}.
    // Si no, devuelve {false, ...}.
    MoveResult try_move(Direction dir, const std::string& player_name);

    // Ataca la celda (x,y). Si hay un NPC lo mata (hardcodeado: muere de un golpe).
    AttackResult attack(const std::string& atacker_name, int x, int y);

    void spawn_npc(NPChostile&& npc);
    bool update_npcs();

    // Snapshot de los NPCs vivos para mandar al cliente via MSG_NPCS_SNAPSHOT
    std::vector<NpcInfo> build_npcs_snapshot() const;

    // Snapshot de los items y oro tirados en el piso, para mandar al cliente
    // via MSG_ITEMS_SNAPSHOT. type = id del item, o "gold" para oro.
    std::vector<ItemFloorInfo> build_items_snapshot() const;
    void spawn_seller(int x, int y);

    // Vender: el player vende item_id al seller adyacente.
    // Devuelve true si la operacion fue exitosa.
    bool player_sell_item(const std::string& player_name, int x, int y,const std::string& item_id);
    // Comprar: el player compra item_id al seller adyacente.
    bool player_buy_item(const std::string& player_name, int x, int y, const std::string& item_id);
    // Lista los items del seller adyacente al player
    std::vector<ItemInfo> list_seller_items(int x, int y);


    std::string sector_of_position(int x, int y);
    void player_equip_item(const std::string& player_name, const std::string& item_id);
    void spawn_player(const std::string& name);
    const Player& get_player(const std::string& name);
    bool player_exists(const std::string& name);  
    positionCoord get_spawn_position();       
    
    std::unique_ptr<Item> pick_up_item(const std::string& player_name);
    void give_item_to_player(const std::string& player_name, std::unique_ptr<Item> item);
    void spawn_item(int x, int y, std::unique_ptr<Item> item);

    void spawn_gold(int x, int y, int amount);
    uint32_t get_player_gold(const std::string& name);
    uint32_t get_player_hp(const std::string& name);
    uint32_t get_player_xp(const std::string& name);
    uint32_t get_player_mana(const std::string& name);
    
    //Player* get_player(const std::string& name);
    
    /*
     * Lee todos los mapas, setea en la matriz map:
     * - los elementos de tipo construccion 
     * - los elementos vacios
     * Recibe el estado inicial del juego (posiciones de players, npcs & items)
     * Setea en sus respectivos vectores la posicion de cada uno
     */
    void init_world(const InitialState& state);
    bool pick_up_gold(const std::string& player_name);
};



#endif
