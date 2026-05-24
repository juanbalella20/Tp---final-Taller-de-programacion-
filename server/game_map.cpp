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
    players.push_back(std::move(player));
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


void GameMap::load_players() {
    // Players are added dynamically via add_player() when clients register.
}

void GameMap::spawn_player(const std::string& name) {
    auto it = spawns.find("player_start");
    int start_x = (it != spawns.end()) ? it->second.x : 1;
    int start_y = (it != spawns.end()) ? it->second.y : 1;
    Player p(name, PlayerRace(), PlayerClass());
    p.update_position(start_x, start_y);
    players.push_back(std::move(p));
    std::cout << "[DEBUG: spawn_player] " << name << " at ("
              << start_x << "," << start_y << ")" << std::endl;
}

// TODO: hacer una clase lectora de TBL!
void GameMap::read_desert() {
    const std::string path = "data/maps/desert/map.toml";

    toml::table tbl;
    try {
        tbl = toml::parse_file(path);
    } catch (const toml::parse_error& e) {
        throw std::runtime_error("GameMap: parse " + path + ": " +
                                 std::string(e.description()));
    }

    width  = tbl["width"].value_or(0);
    height = tbl["height"].value_or(0);
    if (width <= 0 || height <= 0) {
        throw std::runtime_error("GameMap: width/height invlidos en " + path);
    }

    // Matriz [filas=height][columnas=width], todo vaco inicialmente.
    map.assign(height, std::vector<elements>(width, elements::empty));

    // [[collider]] = rectngulos no transitables, en celdas.
    if (auto* arr = tbl["collider"].as_array()) {
        for (auto& node : *arr) {
            auto* t = node.as_table();
            if (!t) continue;
            int cx = (*t)["x"].value_or(0);
            int cy = (*t)["y"].value_or(0);
            int cw = (*t)["w"].value_or(1);
            int ch = (*t)["h"].value_or(1);

            for (int row = cy; row < cy + ch && row < height; ++row) {
                if (row < 0) continue;
                for (int col = cx; col < cx + cw && col < width; ++col) {
                    if (col < 0) continue;
                    map[row][col] = elements::buildings;
                }
            }
        }
    }

    // NPC de prueba hardcodeado: posicion (7,5), cerca del player_start (5,5)
    spawn_npc(7, 5);

    // [[spawn]] = puntos nombrados (player_start, etc.) en celdas.
    spawns.clear();
    if (auto* arr = tbl["spawn"].as_array()) {
        for (auto& node : *arr) {
            auto* t = node.as_table();
            if (!t) continue;
            std::string name = (*t)["name"].value_or<std::string>("");
            if (name.empty()) continue;
            position_coord p{
                (*t)["x"].value_or(0),
                (*t)["y"].value_or(0),
            };
            spawns[name] = p;
        }
    }
}

void GameMap::spawn_npc(int x, int y) {
    if (y >= 0 && y < height && x >= 0 && x < width) {
        map[y][x] = elements::npcs;
        std::cout << "[DEBUG: spawn_npc] NPC at (" << x << "," << y << ")" << std::endl;
    }
}

GameMap::AttackResult GameMap::attack_npc(int x, int y) {
    if (y < 0 || y >= height || x < 0 || x >= width) {
        return {false, false};
    }
    if (map[y][x] != elements::npcs) {
        return {false, false};
    }
    // Hardcodeado: muere de un golpe
    map[y][x] = elements::empty;
    std::cout << "[DEBUG: attack_npc] NPC at (" << x << "," << y << ") killed" << std::endl;
    return {true, true};
}

std::string GameMap::sector_of_position(int x, int y) {
    // to-do: obtener sector segn posicin
    (void)x; (void)y;
    return "desert";
}

const Player& GameMap::get_player(const std::string& name) const {
    for (const auto& player : players) {
        if (player.get_name() == name) {
            return player;
        }
    }
    throw std::runtime_error("Player not found: " + name);
}
// TODO
void GameMap::read_city() {}
// TODO
void GameMap::read_forest() {}
// TODO
void GameMap::read_town() {}
//TODO
void GameMap::set_positions() {}
