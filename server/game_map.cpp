#include "game_map.h"

#include <vector>

GameMap::GameMap() : map(HEIGHT, std::vector<int>(WIDTH, 0)) {}

std::vector<std::vector<int>> GameMap::get_map() {
    return map;
}

void GameMap::add_player(Player player) {
    position_coord coord{player.get_coord_x(), player.get_coord_y()};
    players_coord.insert({player, coord});
}

bool GameMap::is_movement_valid(const ClientCmd cmd) {
    int x = cmd.get_coord_x();
    int y = cmd.get_coord_y();
    if (x >= HEIGHT || y >= WIDTH || x < 0 || y < 0) {
        return false;
    }

    return true;
}

void GameMap::update_position(const ClientCmd cmd) {
    for (auto& [player, coord] : players_coord) {
        if (player.get_name() == cmd.get_player_name()) {
            position_coord new_pos{cmd.get_coord_x(), cmd.get_coord_y()};
            players_coord[player] = new_pos;
            player.update_position(coord.x, coord.y);
        }
    }
}
