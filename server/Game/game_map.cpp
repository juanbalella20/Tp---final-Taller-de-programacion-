#include "game_map.h"
#include "../game_exceptions.h"
#include "item/arma.h"

#include <algorithm>
#include <iostream>
#include <stdexcept>
#include <tuple>
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

GameMap::GameMap() = default;
Zone GameMap::zone_id_of(const std::string& player_name) const {
    auto it = player_zone.find(player_name);
    if (it == player_zone.end()) {
        throw std::runtime_error("Player sin zona asignada: " + player_name);
    }
    return it->second;
}

ZoneWorld& GameMap::zone_of(const std::string& player_name) {
    return zones.at(zone_id_of(player_name));
}

std::vector<const Player*> GameMap::players_in(Zone z) const {
    std::vector<const Player*> result;
    for (const auto& p : players) {
        auto it = player_zone.find(p.get_name());
        if (it != player_zone.end() && it->second == z) {
            result.push_back(&p);
        }
    }
    return result;
}

Zone GameMap::get_player_zone(const std::string& player_name) const {
    return zone_id_of(player_name);
}

std::pair<int, int> GameMap::find_arrival_cell(ZoneWorld& dst, Zone dest_zone) {
    const std::vector<const Player*> dest_players = players_in(dest_zone);
    // Si la zona destino tiene teleport, aparecer adyacente a el.
    if (!dst.get_teleports().empty()) {
        const TeleportDef& dest_tp = dst.get_teleports().front();
        return dst.free_cell_adjacent_to(dest_tp.x, dest_tp.y, dest_players);
    }
    // Si no, cualquier celda libre.
    return dst.find_random_empty_cell(dest_players);
}

TeleportResult GameMap::teleport_player(const std::string& player_name) {
    Player* player = find_player_by_name(player_name);
    if (player == nullptr) return {false, ZONE_DESERT, 0, 0};

    // Chequea si el player esta adyacente a un teleport
    ZoneWorld& src = zone_of(player_name);
    const TeleportDef* tp = src.teleport_adjacent_to(player->get_coord_x(),
                                                     player->get_coord_y());
    if (tp == nullptr) return {false, ZONE_DESERT, 0, 0};

    // Encuentra la zona por nombre
    auto zit = ZONE_NAME_MAP.find(tp->dest_zone);
    if (zit == ZONE_NAME_MAP.end()) return {false, ZONE_DESERT, 0, 0};
    Zone dest_zone = zit->second;

    auto dit = zones.find(dest_zone);
    if (dit == zones.end()) return {false, ZONE_DESERT, 0, 0};  // zona no cargada
    ZoneWorld& dst = dit->second;

    // Ubica al player en una celda libre de la zona destino.
    auto [nx, ny] = find_arrival_cell(dst, dest_zone);
    if (nx == -1) return {false, ZONE_DESERT, 0, 0};  // sin lugar libre

    // Aplica el cambio de zona y posicion.
    player_zone[player_name] = dest_zone;
    player->update_position(nx, ny);
    std::cout << "[DEBUG: teleport] " << player_name << " -> zona "
              << static_cast<int>(dest_zone) << " (" << nx << "," << ny << ")" << std::endl;
    return {true, dest_zone, nx, ny};
}
void GameMap::init_world(const std::map<Zone, std::string>& zone_paths,
                         const std::map<Zone, InitialState>& initial_states) {
    for (const auto& [zone_id, path] : zone_paths) {
        ZoneWorld world;
        world.load_terrain(path);

        auto state_it = initial_states.find(zone_id);
        if (state_it != initial_states.end()) {
            for (const auto& spawn : state_it->second.npcs) {
                world.spawn_npc(make_npc_from_spawn(spawn));
            }
        }

        // Item de prueba hardcodeado. TODO: moverlo a state.items cuando este listo.
        world.spawn_item(7, 7, std::make_unique<Arma>("espada", "espada", 50, 2, 2));
        if (zone_id == ZONE_CITY) {
            std::vector<const Player*> no_players;
            auto [sx, sy] = world.find_random_empty_cell(no_players);
            world.spawn_seller(sx, sy);
            auto [bx, by] = world.find_random_empty_cell(no_players);
            world.spawn_banker(bx, by);
            auto [px, py] = world.find_random_empty_cell(no_players);
            world.spawn_priest(px, py);
        }
        zones.emplace(zone_id, std::move(world));
    }
}

std::vector<std::vector<elements>> GameMap::get_map(const std::string& player_name) {
    return zone_of(player_name).get_map();
}

void GameMap::add_player(Player player) {
    players.push_back(std::move(player));
}

Player* GameMap::find_player_by_name(const std::string& name) {
    auto it = std::find_if(players.begin(), players.end(),
                           [&name](const Player& p) { return p.get_name() == name; });
    if (it == players.end()) return nullptr;
    return &(*it);
}

void GameMap::spawn_player(const std::string& name) {
    // Zona inicial de spawn
    // TODO: derivar de config / persistencia
    //const Zone start_zone = ZONE_CITY;
    const Zone start_zone = ZONE_DESERT;
    player_zone[name] = start_zone;

    // Posicion de spawn hardcodeada
    int start_x = 29;
    int start_y = 15;

    Player player(name, PlayerRace(), PlayerClass());
    player.update_position(start_x, start_y);
    player.add_item(std::make_unique<Arma>("espada", "Espada", 100, 2, 10));
    players.push_back(std::move(player));
    std::cout << "[DEBUG: spawn_player] " << name << " at ("
              << start_x << "," << start_y << ") zona=" << static_cast<int>(start_zone)
              << std::endl;
}

const Player& GameMap::get_player(const std::string& name) {
    Player* player = find_player_by_name(name);
    if (player != nullptr) return *player;
    throw std::runtime_error("Player not found: " + name);
}

bool GameMap::player_exists(const std::string& name) {
    return find_player_by_name(name) != nullptr;
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

    ZoneWorld& world = zone_of(player_name);
    const std::vector<const Player*> here = players_in(zone_id_of(player_name));

    int current_x = player->get_coord_x();
    int current_y = player->get_coord_y();


    int new_x = current_x + dir_to_dx(dir);
    int new_y = current_y + dir_to_dy(dir);

    std::cout << "[DEBUG: try_move " << player_name
              << "] (" << new_x << "," << new_y << ")" << std::endl;

    // Limites del mapa / terreno bloqueado (building)
    if (world.is_blocked_terrain(new_x, new_y)) {
        return {false, player_name, current_x, current_y};
    }
    // Actor en la celda destino.
    if (world.has_actor_at(new_x, new_y, here)) {
        return {false, player_name, current_x, current_y};
    }
    // Item en el piso
    if (world.has_ground_item_at(new_x, new_y)) {
        return {false, player_name, current_x, current_y};
    }

    player->update_position(new_x, new_y);
    return {true, player_name, new_x, new_y};
}

GameMap::AttackResult GameMap::attack(const std::string& attacker_name, int x, int y) {
    Player* attacker = find_player_by_name(attacker_name);
    if (attacker == nullptr) throw AttackerNotFoundException();

    ZoneWorld& world = zone_of(attacker_name);
    const Zone z = zone_id_of(attacker_name);

    // Buscar target en la celda (x,y): primero otros players de la zona, luego NPCs
    Entity* target = nullptr;
    for (auto& p : players) {
        if (p.get_name() == attacker_name) continue;
        auto it = player_zone.find(p.get_name());
        if (it == player_zone.end() || it->second != z) continue;
        if (p.get_coord_x() == x && p.get_coord_y() == y) {
            target = &p;
            break;
        }
    }
    if (target == nullptr) {
        target = world.hostile_at(x, y);
    }
    if (target == nullptr) throw NoEntityException();

    attacker->attack(*target, x, y);

    if (target->is_dead()) {
        // este "1" tiene que cambiar por el oro real que deja el npc/jugador.
        world.spawn_gold(x, y, 1);
        return {true, true, target->get_name()};
    }
    return {true, false, target->get_name()};
}

bool GameMap::update_npcs() {
    bool respawned = false;
    for (auto& [zone_id, world] : zones) {
        if (world.update_npcs()) respawned = true;
    }
    return respawned;
}

std::vector<NpcInfo> GameMap::build_npcs_snapshot(const std::string& player_name) {
    return zone_of(player_name).build_npcs_snapshot();
}

std::vector<ItemFloorInfo> GameMap::build_items_snapshot(const std::string& player_name) {
    return zone_of(player_name).build_items_snapshot();
}

bool GameMap::player_sell_item(const std::string& player_name, int x, int y,
                               const std::string& item_id) {
    Player* player = find_player_by_name(player_name);
    if (player == nullptr) throw std::runtime_error("Player not found: " + player_name);

    NPCseller* seller = zone_of(player_name).seller_at(player->get_coord_x(), player->get_coord_y());
    if (seller == nullptr) throw std::runtime_error("No hay un comerciante adyacente.");

    Command cmd;
    cmd.action = ACTION_SELL;
    cmd.item_id = item_id;
    seller->interact(*player, cmd);
    return true;
}

bool GameMap::player_buy_item(const std::string& player_name, int x, int y,
                              const std::string& item_id) {
    Player* player = find_player_by_name(player_name);
    if (player == nullptr) throw std::runtime_error("Player not found: " + player_name);

    NPCseller* seller = zone_of(player_name).seller_at(player->get_coord_x(), player->get_coord_y());
    if (seller == nullptr) throw std::runtime_error("No hay un comerciante en esa posicion.");

    Command cmd;
    cmd.action = ACTION_BUY;
    cmd.item_id = item_id;
    seller->interact(*player, cmd);
    return true;
}

std::vector<ItemInfo> GameMap::list_seller_items(const std::string& player_name, int x, int y) {
    Player* player = find_player_by_name(player_name);
    if (player == nullptr) throw std::runtime_error("Player not found: " + player_name);
    return zone_of(player_name).list_seller_items(player->get_coord_x(), player->get_coord_y());
}

void GameMap::player_deposit_item(const std::string& player_name, const std::string& item_id) {
    Player* player = find_player_by_name(player_name);
    if (player == nullptr) throw std::runtime_error("Player not found.");
    NPCbanker* banker = zone_of(player_name).banker_adjacent_to(player->get_coord_x(), player->get_coord_y());
    if (banker == nullptr) throw std::runtime_error("No hay un banquero adyacente.");
    Command cmd;
    cmd.action = ACTION_DEPOSIT;
    cmd.item_id = item_id;
    banker->interact(*player, cmd);
}

std::unique_ptr<Item> GameMap::pick_up_item(const std::string& player_name) {
    Player* player = find_player_by_name(player_name);
    if (!player) throw std::runtime_error("Player not found: " + player_name);
    return zone_of(player_name).take_item_near(player->get_coord_x(), player->get_coord_y());
}

bool GameMap::pick_up_gold(const std::string& player_name) {
    Player* player = find_player_by_name(player_name);
    if (!player) throw std::runtime_error("Player not found: " + player_name);
    int amount = zone_of(player_name).take_gold_at(player->get_coord_x(), player->get_coord_y());
    if (amount > 0) {
        player->add_gold(amount);
        return true;
    }
    return false;
}

void GameMap::give_item_to_player(const std::string& player_name, std::unique_ptr<Item> item) {
    Player* player = find_player_by_name(player_name);
    if (!player) throw std::runtime_error("Player not found: " + player_name);
    player->add_item(std::move(item));
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

std::vector<PlayerInfo> GameMap::build_players_snapshot(const std::string& player_name) {
    std::vector<PlayerInfo> snapshot;
    const Zone z = zone_id_of(player_name);
    for (const auto& p : players) {
        if (p.get_name() == player_name) continue;
        auto it = player_zone.find(p.get_name());
        if (it == player_zone.end() || it->second != z) continue;

        snapshot.push_back({p.get_name(), 0, 0, p.get_coord_x(), p.get_coord_y()});
    }
    return snapshot;
}
