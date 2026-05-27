#include "game_map.h"
#include "game_exceptions.h"
#include "arma.h"

#include <algorithm>
#include <iostream>
#include <stdexcept>
#include <vector>

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

    player->update_position(new_x, new_y);
    return {true, player_name, new_x, new_y};
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

    // NPC de prueba hardcodeado: posicion (7,5), cerca del player_start (5,5)
    spawn_npc(7, 5);
    // NPC comerciante de prueba hardcodeado
    spawn_seller(6, 5);

    // [[spawn]] = puntos nombrados (player_start, etc.) en celdas.
    spawns = md.get_spawns();
}


void GameMap::spawn_npc(int x, int y) {
    if (y >= 0 && y < height && x >= 0 && x < width) {
        NPChostile npc;
        npc.set_goblin(1);
        npc.set_position(x, y);
        npcs.push_back(std::move(npc));
        map[y][x] = elements::npcs;
        std::cout << "[DEBUG: spawn_npc] NPChostile at (" << x << "," << y << ")" << std::endl;
    }
}

void GameMap::spawn_seller(int x, int y) {
    if (y >= 0 && y < height && x >= 0 && x < width) {
        sellers.emplace_back(x, y);
        map[y][x] = elements::npcs;
        std::cout << "[DEBUG: spawn_seller] NPCseller at (" << x << "," << y << ")" << std::endl;
    }
}
 
bool GameMap::player_sell_item(const std::string& player_name, int x, int y, const std::string& item_id) {
    Player* player = find_player_by_name(player_name);
    if (player == nullptr) throw std::runtime_error("Player not found: " + player_name);
 
    NPCseller* seller = nullptr;
    for (auto& s : sellers) {
        if (s.get_coord_x() == x && s.get_coord_y() == y) {
            seller = &s;
            break;
        }
    }
    if (seller == nullptr) throw std::runtime_error("No hay un comerciante en esa posicion.");
 
    Command cmd;
    cmd.item_id = item_id;
    seller->interact(*player, cmd);
 
    return true;
}
 
std::vector<ItemInfo> GameMap::list_seller_items(int x, int y) {
    NPCseller* seller = nullptr;
    for (auto& s : sellers) {
        if (s.get_coord_x() == x && s.get_coord_y() == y) {
            seller = &s;
            break;
        }
    }
    if (seller == nullptr) throw std::runtime_error("No hay un comerciante en esa posicion.");
    return seller->list_items();
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
