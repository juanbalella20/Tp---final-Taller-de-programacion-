#ifndef GAME_MAP_H_
#define GAME_MAP_H_

#include "../common/clientCmd.h"
#include "player.h"
#include "npcHostile.h"
#include "../common/game_constants.h"
#include "../common/item_info.h"
#include "../common/mapLoader.h"

#include <vector>
#include <map>
#include <string>
#include <utility>

struct InitialState {
    std::vector<NPChostile> npcs;
    std::vector<Item> items;
};

class GameMap {

private:
    int width = 0;
    int height = 0;
    std::vector<std::vector<elements>> map;
    std::vector<Player> players;
    std::vector<NPChostile> npcs;
    std::vector<ItemInfo> items;
    std::map<std::string, position_coord> spawns;
    std::map<std::string, position_coord> player_position;
    //TileGrid tiles;

    Player* find_player_by_name(const std::string& name);
    bool look_for_entity(int x, int y);
    Entity* find_entity_at(int x, int y);
    std::pair<int,int> find_random_empty_cell();
    

    sectorPerimiter forest_perimiter;
    sectorPerimiter town_perimiter;
    sectorPerimiter city_perimiter;
    sectorPerimiter desert_perimiter;

public:
    GameMap();

    std::vector<std::vector<elements>> get_map();

    int get_width()  const { return width; }
    int get_height() const { return height; }

    const std::map<std::string, position_coord>& get_spawns() const { return spawns; }

    void add_player(Player player);

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

    std::string sector_of_position(int x, int y);
    // Lee el archivo (TOML) de desierto y devuelve un tipo MapLoader
    MapLoader read_desert();
    void read_city();
    void read_forest();
    void read_town();
    void player_equip_item(const std::string& player_name, const std::string& item_id);
    // Lee archivo de persistencia
    void set_positions();
    void load_players();
    void spawn_player(const std::string& name);
    const Player& get_player(const std::string& name);
    bool player_exists(const std::string& name);  
    position_coord get_spawn_position();           

    //Player* get_player(const std::string& name);
    
    /*
     * Lee todos los mapas, setea en la matriz map:
     * - los elementos de tipo construccion 
     * - los elementos vacios
     * Recibe el estado inicial del juego (posiciones de players, npcs & items)
     * Setea en sus respectivos vectores la posicion de cada uno
     */
    void init_world(const InitialState& state);
};



#endif
