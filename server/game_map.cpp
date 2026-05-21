#include "game_map.h"

#include <stdexcept>
#include <vector>
#include <iostream>

#include "../vendored/tomlplusplus/toml.hpp"

GameMap::GameMap() : width(0), height(0), map() {}

std::vector<std::vector<elements>> GameMap::get_map() {
    return map;
}

void GameMap::add_player(Player player) {
    players.push_back(player);
}

// TODO: refactorizar funcion
static int dir_to_dx(Direction dir) {
    switch (dir) {
        case Direction::DIR_EAST: return 1;
        case Direction::DIR_WEST: return -1;
        default: return 0;
    }
}
// TODO: refactorizar funcion
static int dir_to_dy(Direction dir) {
    // En pantalla Y crece hacia abajo: NORTH disminuye y, SOUTH la aumenta.
    switch (dir) {
        case Direction::DIR_NORTH: return -1;
        case Direction::DIR_SOUTH: return 1;
        default: return 0;
    }
}

GameMap::MoveResult GameMap::try_move(Direction dir, const std::string& player_name) {
    for (auto& player : players) {
        if (player.get_name() != player_name) continue;

        int new_x = player.get_coord_x() + dir_to_dx(dir);
        int new_y = player.get_coord_y() + dir_to_dy(dir);

        std::cout << "[DEBUG: try_move " << player_name
                  << "] (" << new_x << "," << new_y << ")" << std::endl;

        // x es columna (width), y es fila (height). map se indexa [y][x].
        if (new_x < 0 || new_y < 0 || new_x >= width || new_y >= height) {
            return {false, player_name, 0, 0};
        }
        if (map[new_y][new_x] != elements::empty) {
            return {false, player_name, 0, 0};
        }

        player.update_position(new_x, new_y);
        return {true, player_name, new_x, new_y};
    }
    return {false, player_name, 0, 0};
}

std::string GameMap::sector_of_position(int x, int y) {
    // to-do: obtener sector segn posicin
    (void)x; (void)y;
    return "desert";
}

// TODO
void GameMap::read_city() {}
// TODO
void GameMap::read_forest() {}
// TODO
void GameMap::read_town() {}
//TODO
void GameMap::set_positions() {}
