#ifndef GAME_MAP_H_
#define GAME_MAP_H_

#include "../common/clientCmd.h"
#include "../common/player.h"

#include <vector>
#include <map>

#define HEIGHT 8
#define WIDTH 8

struct position_coord {
    int x;
    int y;
};

struct sectorPerimiter {
    position_coord top_left;
    position_coord top_right;
    position_coord bottom_left;
    position_coord bottom_right;
};

enum class elements {
    players,
    npcs,
    objects,
    buildings,
    empty
};

class GameMap {

private:
    std::vector<std::vector<elements>> map;
    std::vector<Player> players;

    sectorPerimiter forest_perimiter;
    sectorPerimiter town_perimiter;
    sectorPerimiter city_perimiter;
    sectorPerimiter desert_perimiter;

public:
    GameMap();

    std::vector<std::vector<elements>> get_map();

    void add_player(Player player);

    bool is_movement_valid(const ClientCmd cmd);

    void update_position(const ClientCmd cmd);

    std::string sector_of_position(int x, int y);
};



#endif
