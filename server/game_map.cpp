#include "game_map.h"
#include "game_exceptions.h"
#include "arma.h"

#include <algorithm>
#include <iostream>
#include <stdexcept>
#include <vector>
#include <cstdlib>

#include "../common/mapLoader.h"

GameMap::GameMap() : width(0), height(0), map() {}

std::vector<std::vector<elements>> GameMap::get_map() {
    return map;
}

void GameMap::add_player(Player player) {
    players.push_back(std::move(player));
}
 Player* GameMap::find_player_by_name(const std::string& name)  {
    auto it = std::find_if(players.begin(), players.end(),
                           [&name](const Player& p) { return p.get_name() == name; });
    if (it == players.end()) return nullptr;
    return &(*it);
}

void GameMap::player_equip_item(const std::string& player_name, const std::string& item_id) {
    Player* player = find_player_by_name(player_name);
    if (player == nullptr) {
        throw std::runtime_error("Player not found: " + player_name);
    }
    player->equip_item(item_id);
    // TODO: equipar item por id cuando Player::equip_item esté implementado.
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
    Player* player = find_player_by_name(player_name);
    if (player == nullptr) {
        return {false, player_name, 0, 0};
    }

    int new_x = player->get_coord_x() + dir_to_dx(dir);
    int new_y = player->get_coord_y() + dir_to_dy(dir);

    std::cout << "[DEBUG: try_move " << player_name
              << "] (" << new_x << "," << new_y << ")" << std::endl;

    // x es columna (width), y es fila (height). map se indexa [y][x].
    if (new_x < 0 || new_y < 0 || new_x >= width || new_y >= height) {
        return {false, player_name, 0, 0};
    }
    if (map[new_y][new_x] != elements::empty) {
        return {false, player_name, 0, 0};
    }
    map[player->get_coord_y()][player->get_coord_x()] = elements::empty;
    map[new_y][new_x] = elements::players;

    player->update_position(new_x, new_y);
    return {true, player_name, new_x, new_y};
}


void GameMap::load_players() {
    // Players are added dynamically via add_player() when clients register.
}

void GameMap::spawn_player(const std::string& name) {
    auto [x, y] = find_random_empty_cell();
    /*
    auto it = spawns.find("player_start");
    int start_x = (it != spawns.end()) ? it->second.x : 1;
    int start_y = (it != spawns.end()) ? it->second.y : 1;
    */
    int start_x = x != -1 ? x : 1;
    int start_y = y != -1 ? y : 1;
    Player p(name, PlayerRace(), PlayerClass());
    map[start_y][start_x] = elements::players;

    p.update_position(start_x, start_y);
    p.add_item(std::make_unique<Arma>("espada", "Espada", 100, 2, 10));
    players.push_back(std::move(p));
    std::cout << "[DEBUG: spawn_player] " << name << " at ("
              << start_x << "," << start_y << ")" << std::endl;
}

void GameMap::read_desert() {
    MapLoader md;
    md.load("data/maps/desert/map.toml");

    width  = md.get_width();
    height = md.get_height();
    map.assign(height, std::vector<elements>(width, elements::empty));

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            if (md.is_collidable(x, y)) {
                map[y][x] = elements::buildings;
            }
        }
    }

    // NPCs de prueba: goblin respawnea en 5s, spider en 2s
    NPChostile goblin("Goblin", 30, 5, 100);
    goblin.set_position(7, 5);
    spawn_npc(goblin);

    NPChostile spider("Spider", 20, 4, 40);
    spider.set_position(9, 5);
    spawn_npc(spider);

    // [[spawn]] = puntos nombrados (player_start, etc.) en celdas.
    spawns = md.get_spawns();
}


void GameMap::spawn_npc(NPChostile npc) {
    int x = npc.get_coord_x();
    int y = npc.get_coord_y();
    if (y >= 0 && y < height && x >= 0 && x < width) {
        map[y][x] = elements::npcs;
        npcs.push_back(std::move(npc));
        std::cout << "[DEBUG: spawn_npc] " << npcs.back().get_name()
                  << " at (" << x << "," << y << ")" << std::endl;
    }
}


std::pair<int,int> GameMap::find_random_empty_cell() {
    std::vector<std::pair<int,int>> empty_cells;
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            if (map[y][x] == elements::empty) {
                empty_cells.emplace_back(x, y);
            }
        }
    }
    if (empty_cells.empty()) return {-1, -1};
    return empty_cells[rand() % empty_cells.size()];//mejorar random
}

bool GameMap::update_npcs() {
    bool respawned = false;
    for (auto& npc : npcs) {
        if (!npc.is_dead()) continue;
        npc.reduce_ticks_to_spawn();
        if (!npc.can_spawn()) continue;

        auto [rx, ry] = find_random_empty_cell();
        if (rx == -1) continue;

        npc.revive(rx, ry);
        map[ry][rx] = elements::npcs;
        std::cout << "[DEBUG: update_npcs] " << npc.get_name()
                  << " respawned at (" << rx << "," << ry << ")" << std::endl;
        respawned = true;
    }
    return respawned;
}

bool GameMap::look_for_entity(int x, int y) {
    if (x < 0 || x >= width || y < 0 || y >= height) return false;
    return map[y][x] == elements::npcs || map[y][x] == elements::players;
}

Entity* GameMap::find_entity_at(int x, int y) {
    if (x < 0 || x >= width || y < 0 || y >= height) return nullptr;

    if (map[y][x] == elements::players) {
        for (auto& player : players) {
            if (player.get_coord_x() == x && player.get_coord_y() == y) {
                return &player;
            }
        }
    }

    if (map[y][x] == elements::npcs) {
        for (auto& npc : npcs) {
            if (npc.get_coord_x() == x && npc.get_coord_y() == y) {
                return &npc;
            }
        }
    }

    return nullptr;
}

GameMap::AttackResult GameMap::attack(const std::string& attacker_name, int x, int y) {
    if (!look_for_entity(x, y)) throw NoEntityException();

    Player* attacker = find_player_by_name(attacker_name);
    if (attacker == nullptr) throw AttackerNotFoundException();

    Entity* target = find_entity_at(x, y);
    if (target == nullptr) throw NoEntityException();

    attacker->attack(*target, x, y);

    if (target->is_dead()) {
        map[y][x] = elements::empty;
        return {true, true, target->get_name()};
    }
    return {true, false, target->get_name()};
}

std::string GameMap::sector_of_position(int x, int y) {
    // to-do: obtener sector segn posicin
    (void)x; (void)y;
    return "desert";
}

const Player& GameMap::get_player(const std::string& name) {
    Player* player = find_player_by_name(name);
    if (player != nullptr) return *player;
    throw std::runtime_error("Player not found: " + name);
}

bool GameMap::player_exists(const std::string& name) {
    return find_player_by_name(name) != nullptr;
}
// TODO
void GameMap::read_city() {}
// TODO
void GameMap::read_forest() {}
// TODO
void GameMap::read_town() {}
//TODO
void GameMap::set_positions() {}

/*Player* GameMap::get_player(const std::string& name) {
    for (auto& player : players) {
        if (player.get_name() == name) return &player;
    }
    return nullptr;
}

bool GameMap::player_exists(const std::string& name) {
    return get_player(name) != nullptr;
}

position_coord GameMap::get_spawn_position() {
    auto it = spawns.find("player_start");
    if (it != spawns.end()) return it->second;
    // Fallback: primera celda libre del mapa
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            if (map[y][x] == elements::empty) return {x, y};
        }
    }
    return {1, 1};
}
*/
