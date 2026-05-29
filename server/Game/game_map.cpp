#include "game_map.h"
#include "../game_exceptions.h"
#include "item/arma.h"

#include <algorithm>
#include <iostream>
#include <stdexcept>
#include <vector>
#include <cstdlib>

// Esto tienen que ser posiciones aleatorias
NPChostile make_npc_from_spawn(const NpcSpawn& spawn) {
    // Catalogo de tipos de NPC hostiles. Mas adelante esto puede vivir
    // en un archivo de configuracion o base de datos.
    if (spawn.type == "goblin") {
        NPChostile npc("goblin", "Goblin", 30, 5, 100);
        npc.set_position(spawn.x, spawn.y);
        return npc;
    }
    if (spawn.type == "spider") {
        NPChostile npc("spider", "Spider", 20, 4, 40);
        npc.set_position(spawn.x, spawn.y);
        return npc;
    }
    // Fallback para tipos desconocidos.
    NPChostile npc(spawn.type, spawn.type, 10, 1, 50);
    npc.set_position(spawn.x, spawn.y);
    return npc;
}

GameMap::GameMap() : width(0), height(0), map() {}

std::vector<std::vector<elements>> GameMap::get_map() {
    return map;
}

void GameMap::init_world(const InitialState& state) {
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
    // [[spawn]] = puntos nombrados (player_start, etc.) en celdas.
    spawns = md.get_spawns();

    // itera sobre las posiciones de los actores
    for (const auto& spawn : state.npcs) {
        spawn_npc(make_npc_from_spawn(spawn));
    }
    spawn_seller(1,1);

    // Los players se crean en MSG_REGISTER, no desde el InitialState.

    // Item de prueba hardcodeado. TODO: moverlo a state.items cuando este listo.
    spawn_item(7, 7, std::make_unique<Arma>("espada", "espada", 50, 2, 2));
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

// Devuelve true si hay un actor (player o NPC vivo) en (x,y).
bool GameMap::has_actor_at(int x, int y) {
    for (const auto& p : players) {
        if (p.get_coord_x() == x && p.get_coord_y() == y) return true;
    }
    for (const auto& n : npcs) {
        if (!n.is_dead() && n.get_coord_x() == x && n.get_coord_y() == y) return true;
    }
    return false;
}

GameMap::MoveResult GameMap::try_move(Direction dir, const std::string& player_name) {
    Player* player = find_player_by_name(player_name);
    if (player == nullptr) {
        return {false, player_name, 0, 0};
    }

    int current_x = player->get_coord_x();
    int current_y = player->get_coord_y();

    int new_x = current_x + dir_to_dx(dir);
    int new_y = current_y + dir_to_dy(dir);

    std::cout << "[DEBUG: try_move " << player_name
              << "] (" << new_x << "," << new_y << ")" << std::endl;

    // Limites del mapa.
    if (new_x < 0 || new_y < 0 || new_x >= width || new_y >= height) {
        return {false, player_name, current_x, current_y};
    }
    // Terreno bloqueado (edificio).
    if (map[new_y][new_x] != elements::empty) {
    // VER
        return {false, player_name, current_x, current_y};
    }
    // Actor en la celda destino.
    if (has_actor_at(new_x, new_y)) {
        return {false, player_name, current_x, current_y};
    }

    for (const auto& gi : ground_items) {
        if (gi.pos.x == new_x && gi.pos.y == new_y) {
            return {false, player_name, current_x, current_y};
        }
    }

    player->update_position(new_x, new_y);
    return {true, player_name, new_x, new_y};
}


void GameMap::spawn_player(const std::string& name) {
    auto [x, y] = find_random_empty_cell();
    int start_x = x != -1 ? x : 1;
    int start_y = y != -1 ? y : 1;
    start_x = 29;
    start_y = 15;
    Player player(name, PlayerRace(), PlayerClass());
    player.update_position(start_x, start_y);
    player.add_item(std::make_unique<Arma>("espada", "Espada", 100, 2, 10));
    players.push_back(std::move(player));
    std::cout << "[DEBUG: spawn_player] " << name << " at ("
              << start_x << "," << start_y << ")" << std::endl;
}

void GameMap::spawn_npc(NPChostile&& npc) {
    int x = npc.get_coord_x();
    int y = npc.get_coord_y();
    if (y >= 0 && y < height && x >= 0 && x < width) {
        npcs.push_back(std::move(npc));
        std::cout << "[DEBUG: spawn_npc] " << npcs.back().get_name()
                  << " at (" << x << "," << y << ")" << std::endl;
    }
}


std::pair<int,int> GameMap::find_random_empty_cell() {
    std::vector<std::pair<int,int>> empty_cells;
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            if (map[y][x] == elements::empty && !has_actor_at(x, y)) {
                empty_cells.emplace_back(x, y);
            }
        }
    }
    if (empty_cells.empty()) return {-1, -1};
    return empty_cells[rand() % empty_cells.size()];//mejorar random
}

std::vector<ItemFloorInfo> GameMap::build_items_snapshot() const {
    std::vector<ItemFloorInfo> snapshot;
    snapshot.reserve(ground_items.size() + ground_gold.size());
    for (const auto& gi : ground_items) {
        if (!gi.item) continue;
        snapshot.emplace_back(gi.item->get_id(), gi.pos.x, gi.pos.y);
    }
    for (const auto& gg : ground_gold) {
        snapshot.emplace_back("gold", gg.pos.x, gg.pos.y);
    }
    return snapshot;
}

std::vector<NpcInfo> GameMap::build_npcs_snapshot() const {
    std::vector<NpcInfo> snapshot;
    snapshot.reserve(npcs.size());
    for (const auto& npc : npcs) {
        if (npc.is_dead()) continue;
        NpcInfo npcinfo;
        npcinfo.name = npc.get_name();
        npcinfo.type = npc.get_type_id();
        npcinfo.x = npc.get_coord_x();
        npcinfo.y = npc.get_coord_y();
        snapshot.push_back(npcinfo);
    }
    return snapshot;
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
        std::cout << "[DEBUG: update_npcs] " << npc.get_name()
                  << " respawned at (" << rx << "," << ry << ")" << std::endl;
        respawned = true;
    }
    return respawned;
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
    cmd.action = ACTION_SELL;
    cmd.item_id = item_id;
    seller->interact(*player, cmd);
 
    return true;
}

bool GameMap::player_buy_item(const std::string& player_name, int x, int y, const std::string& item_id) {
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
    cmd.action = ACTION_BUY;
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
    return find_entity_at(x, y) != nullptr;
}

Entity* GameMap::find_entity_at(int x, int y) {
    if (x < 0 || x >= width || y < 0 || y >= height) return nullptr;

    for (auto& player : players) {
        if (player.get_coord_x() == x && player.get_coord_y() == y) {
            return &player;
        }
    }
    for (auto& npc : npcs) {
        if (!npc.is_dead() && npc.get_coord_x() == x && npc.get_coord_y() == y) {
            return &npc;
        }
    }
    return nullptr;
}

void GameMap::spawn_gold(int x, int y, int amount) {
    if (y >= 0 && y < height && x >= 0 && x < width) {
        positionCoord coord{x, y};
        ground_gold.push_back({coord, amount});
        std::cout << "[DEBUG: spawn_gold] at (" << x << "," << y << ")" << std::endl;
    }
}

GameMap::AttackResult GameMap::attack(const std::string& attacker_name, int x, int y) {
    Player* attacker = find_player_by_name(attacker_name);
    if (attacker == nullptr) throw AttackerNotFoundException();

    Entity* target = find_entity_at(x, y);
    if (target == nullptr) throw NoEntityException();

    attacker->attack(*target, x, y);

    if (target->is_dead()) {
        // este "1" tiene que cambiar por el oro real
        // que deja el npc/jugador atacado !
        spawn_gold(x, y, 1);
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
static bool is_adyacent(const positionCoord& a, const positionCoord& b) {
    return std::abs(a.x - b.x) + std::abs(a.y - b.y) <= 1;
}

std::unique_ptr<Item> GameMap::pick_up_item(const std::string& player_name) {
    Player* player = find_player_by_name(player_name);
    if (!player) throw std::runtime_error("Player not found: " + player_name);

    positionCoord player_pos{player->get_coord_x(), player->get_coord_y()};

    // Items: cualquiera adyacente (incluyendo mismo tile).
    auto item_it = std::find_if(ground_items.begin(), ground_items.end(),
        [&player_pos](const groundItem& g_item) {
            return is_adyacent(g_item.pos, player_pos);
        });

    if (item_it != ground_items.end()) {
        auto item = std::move(item_it->item);
        ground_items.erase(item_it);
        return item;
    }
    return nullptr;
}

bool GameMap::pick_up_gold(const std::string& player_name) {
    Player* player = find_player_by_name(player_name);
    if (!player) throw std::runtime_error("Player not found: " + player_name);
    positionCoord player_pos{player->get_coord_x(), player->get_coord_y()};

    // Oro: solo si esta exactamente debajo del player.
    auto gold_it = std::find_if(ground_gold.begin(), ground_gold.end(),
        [&player_pos](const groundGold& g_gold) {
            return g_gold.pos.x == player_pos.x && g_gold.pos.y == player_pos.y;
        });

    if (gold_it != ground_gold.end()) {
        player->add_gold(gold_it->amount);
        ground_gold.erase(gold_it);
        return true;
    }
    return false;
}

void GameMap::give_item_to_player(const std::string& player_name, std::unique_ptr<Item> item) {
    Player* player = find_player_by_name(player_name);
    if (!player) throw std::runtime_error("Player not found: " + player_name);
    player->add_item(std::move(item));
}

void GameMap::spawn_item(int x, int y, std::unique_ptr<Item> item) {
    if (y >= 0 && y < height && x >= 0 && x < width) {
        std::string id = item->get_id();
        positionCoord coord{x, y};
        ground_items.push_back({coord, std::move(item)});
        std::cout << "[DEBUG: spawn_item] " << id << " at (" << x << "," << y << ")" << std::endl;
    }
}

std::vector<PlayerInfo> GameMap::build_player_snapshot(const std::string& player_name) {
    std::vector<PlayerInfo> snapshot;
    for (const auto& p : players) {
        if (p.get_name() == player_name) continue;
        snapshot.push_back({p.get_name(), 0, 0, p.get_coord_x(), p.get_coord_y()});
    }
    return snapshot;
}

uint32_t GameMap::get_player_gold(const std::string& name) {
    Player* player = find_player_by_name(name);
    return player->get_gold();
}

uint32_t GameMap::get_player_hp(const std::string& name) {
    Player* player = find_player_by_name(name);
    return player->get_lives();
}

uint32_t GameMap::get_player_xp(const std::string& name) {
    Player* player = find_player_by_name(name);
    return player->get_xp();
}

uint32_t GameMap::get_player_mana(const std::string& name) {
    Player* player = find_player_by_name(name);
    return player->get_mana();
}

