#ifndef GAME_MAP_H_
#define GAME_MAP_H_

#include "../common/clientCmd.h"
#include "player.h"
#include "npcHostile.h"
#include "../common/game_constants.h"

#include <vector>
#include <map>
#include <string>
#include <utility>



class GameMap {

private:
    int width = 0;
    int height = 0;
    std::vector<std::vector<elements>> map;
    std::vector<Player> players;
    std::vector<NPChostile> npcs;
    std::map<std::string, positionCoord> spawns;
    std::vector<groundItem> ground_items;
    std::vector<groundGold> ground_gold;

    Player* find_player_by_name(const std::string& name);
    bool look_for_entity(int x, int y);
    Entity* find_entity_at(int x, int y);

    sectorPerimiter forest_perimiter;
    sectorPerimiter town_perimiter;
    sectorPerimiter city_perimiter;
    sectorPerimiter desert_perimiter;

public:
    GameMap();

    std::vector<std::vector<elements>> get_map();

    int get_width()  const { return width; }
    int get_height() const { return height; }

    const std::map<std::string, positionCoord>& get_spawns() const { return spawns; }

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

    void spawn_npc(int x, int y);

    std::string sector_of_position(int x, int y);
    void read_desert();
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
    positionCoord get_spawn_position();       
    
    std::unique_ptr<Item> pick_up_item(const std::string& player_name);
    void give_item_to_player(const std::string& player_name, std::unique_ptr<Item> item);
    void spawn_item(int x, int y, std::unique_ptr<Item> item);

    void spawn_gold(int x, int y, int amount);
    
    //Player* get_player(const std::string& name);
};



#endif
