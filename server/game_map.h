#ifndef GAME_MAP_H_
#define GAME_MAP_H_

#include "../common/clientCmd.h"
#include "../common/player.h"
#include "../common/game_constants.h"

#include <vector>
#include <map>
#include <string>



class GameMap {

private:
    int width = 0;
    int height = 0;
    std::vector<std::vector<elements>> map;
    std::vector<Player> players;
    std::map<std::string, position_coord> spawns;

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
        bool npc_died;
    };

    // Calcula la nueva posicion del player a partir de su posicion actual
    // y la direccion. Si es valida, la aplica y devuelve {true, name, x, y}.
    // Si no, devuelve {false, ...}.
    MoveResult try_move(Direction dir, const std::string& player_name);

    // Ataca la celda (x,y). Si hay un NPC lo mata (hardcodeado: muere de un golpe).
    AttackResult attack_npc(int x, int y);

    void spawn_npc(int x, int y);

    std::string sector_of_position(int x, int y);
    void read_desert();
    void read_city();
    void read_forest();
    void read_town();
    // Lee archivo de persistencia
    void set_positions();
    void load_players();
    void spawn_player(const std::string& name);
};



#endif
