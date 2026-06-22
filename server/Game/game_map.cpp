#include "game_map.h"
#include "../game_exceptions.h"
#include "item/item_catalog.h"
#include "game_config.h"

#include <algorithm>
#include <iostream>
#include <stdexcept>
#include <tuple>
#include <vector>
#include <cstdlib>
#include <cmath>
#include <climits>

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

std::vector<Player*> GameMap::players_in(Zone z) {
    std::vector<Player*> result;
    for (auto& p : players) {
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

std::pair<int, int> GameMap::find_arrival_cell(ZoneWorld& dst, Zone dest_zone, Zone src_zone) {
    const std::vector<Player*> dest_players = players_in(dest_zone);
    // Si la zona destino tiene teleports, aparecer adyacente al que apunta de
    // vuelta a la zona de origen (el "par" del que se acaba de usar). Si
    // ninguno apunta ahi (p.ej. /tp a una zona sin camino de vuelta), se cae
    // al primero de la lista.
    const auto& tps = dst.get_teleports();
    if (!tps.empty()) {
        const TeleportDef* dest_tp = &tps.front();
        for (const TeleportDef& tp : tps) {
            auto it = ZONE_NAME_MAP.find(tp.dest_zone);
            if (it != ZONE_NAME_MAP.end() && it->second == src_zone) {
                dest_tp = &tp;
                break;
            }
        }
        return dst.free_cell_adjacent_to(dest_tp->x, dest_tp->y, dest_players);
    }
    // Si no, cualquier celda libre.
    return dst.find_random_empty_cell(dest_players);
}

TeleportResult GameMap::force_zone_change(const std::string& player_name,
                                          Zone dest_zone) {
    Player* player = find_player_by_name(player_name);
    if (player == nullptr) return {false, ZONE_DESERT, 0, 0};

    auto dit = zones.find(dest_zone);
    if (dit == zones.end()) return {false, ZONE_DESERT, 0, 0};  // zona no cargada
    ZoneWorld& dst = dit->second;

    // Zona de origen ANTES de re-etiquetar: decide a que teleport de la zona
    // destino se llega, y el gameloop la usa para avisar a los que quedan en
    // esa zona que el player se fue.
    Zone src_zone = zone_id_of(player_name);

    auto [nx, ny] = find_arrival_cell(dst, dest_zone, src_zone);
    if (nx == -1) return {false, ZONE_DESERT, 0, 0};  // sin lugar libre

    player_zone[player_name] = dest_zone;
    player->update_position(nx, ny);
    return {true, dest_zone, nx, ny, src_zone};
}
void GameMap::init_world(const std::map<Zone, ZoneSpawnConfig>& zone_configs) {
    for (const auto& [zone_id, cfg] : zone_configs) {
        ZoneWorld world;
        world.init(cfg);  // carga su terreno y se puebla a sí misma
        zones.emplace(zone_id, std::move(world));
    }
}

std::vector<std::vector<elements>> GameMap::get_map(const std::string& player_name) {
    return zone_of(player_name).get_map();
}

void GameMap::add_player(Player player) {
    players.push_back(std::move(player));
}

void GameMap::remove_player(const std::string& name) {
    players.erase(std::remove_if(players.begin(), players.end(),
                                 [&name](const Player& p) { return p.get_name() == name; }),
                  players.end());
    player_zone.erase(name);
}

void GameMap::add_persisted_player(Player player, Zone zone) {
    player_zone[player.get_name()] = zone;
    players.push_back(std::move(player));
}

Player* GameMap::find_player_by_name(const std::string& name) {
    auto it = std::find_if(players.begin(), players.end(),
                           [&name](const Player& p) { return p.get_name() == name; });
    if (it == players.end()) return nullptr;
    return &(*it);
}

void GameMap::spawn_player(const std::string& name, const std::string& race, const std::string& pclass) {
    // Zona inicial de spawn
    const Zone start_zone = ZONE_CITY;
    player_zone[name] = start_zone;
    const auto& cfg = GameConfig::instance();

    std::pair<int, int> spawn_pos = zone_of(name).find_random_empty_cell(players_in(start_zone));
    int start_x = spawn_pos.first;
    int start_y = spawn_pos.second;

    PlayerRace player_race = PlayerRace::from_name(race);
    PlayerClass player_class = PlayerClass::from_name(pclass);

    Player player(name, player_race, player_class);
    player.update_position(start_x, start_y);
    // Inventario inicial: la lista de ids vive en [player].initial_inventory
    // (config.toml); el catalogo es la unica fuente de verdad de los stats.
    ItemCatalog catalog;
    for (const std::string& id : GameConfig::instance().initial_inventory) {
        if (auto item = catalog.make_item(id)) {
            player.add_item(std::move(item));
        }
    }
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

Player* GameMap::get_player_mut(const std::string& name) {
    return find_player_by_name(name);
}

bool GameMap::player_exists(const std::string& name) {
    return find_player_by_name(name) != nullptr;
}

bool GameMap::player_equip_item(const std::string& player_name, uint64_t item_uid) {
    Player* player = find_player_by_name(player_name);
    if (player == nullptr) {
        throw std::runtime_error("Player not found: " + player_name);
    }
    player->equip_item(item_uid);
    return player->has_weapon_equipped();
}
int GameMap::dir_to_dx(Direction dir) {
    switch (dir) {
        case Direction::DIR_EAST: return 1;
        case Direction::DIR_WEST: return -1;
        default: return 0;
    }
}
int GameMap::dir_to_dy(Direction dir) {
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
    const std::vector<Player*> here = players_in(zone_id_of(player_name));

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
    // Item en el piso: bloquea solo a los jugadores vivos. Un fantasma
    // (jugador muerto) puede pasar por encima de cualquier item.
    // if (!player->is_ghost() && world.has_ground_item_at(new_x, new_y)) {
    //     return {false, player_name, current_x, current_y};
    // }

    player->update_position(new_x, new_y);
    player_zone[player_name] = zone_id_of(player_name);
    return {true, player_name, new_x, new_y};
}

bool GameMap::same_clan(Player* player1, Player* player2) {
    std::string name1 = player1->get_name();
    std::string name2 = player2->get_name();

    auto clan = std::find_if(clans.begin(), clans.end(), 
        [&name1, &name2](auto& par) {
            return par.second.same_clan(name1, name2);
        }
    );

    if (clan != clans.end()) {
        return true;
    }

    return false;
}

std::string GameMap::clan_of(const std::string& player_name) const {
    for (const auto& [name, clan] : clans) {
        if (clan.joined(player_name)) {
            return name;
        }
    }
    return "";
}

GameMap::AttackResult GameMap::attack(const std::string& attacker_name, int x, int y) {
    Player* attacker = find_player_by_name(attacker_name);
    if (attacker == nullptr) throw AttackerNotFoundException();

    ZoneWorld& world = zone_of(attacker_name);
    const Zone z = zone_id_of(attacker_name);

    // Buscar target en la celda (x,y): primero otros players de la zona, luego NPCs
    Entity* target = nullptr;
    Player* target_player = nullptr;
    for (auto& p : players) {
        if (p.get_name() == attacker_name) continue;
        auto it = player_zone.find(p.get_name());
        if (it == player_zone.end() || it->second != z) continue;
        if (p.get_coord_x() == x && p.get_coord_y() == y) {
            target = &p;
            target_player = &p;
            break;
        }
    }
    if (target == nullptr) {
        target = world.hostile_at(x, y);
    }
    if (target == nullptr) throw NoEntityException();

    bool target_is_player = (target_player != nullptr);

    // Fair play: solo aplica en PvP
    if (target_is_player) {
        // Zona segura (config.toml: zones.<nombre>.safe_zone): sin PvP para
        if (GameConfig::instance().is_safe_zone(z))
            throw AttackNotAllowedException("Estas en una zona segura: no se puede atacar a otros jugadores");
        if (attacker->is_newbie() || target_player->is_newbie())
            throw AttackNotAllowedException("Los newbies no pueden atacar ni ser atacados");
        if (!attacker->can_attack_level(target_player->get_level()))
            throw AttackNotAllowedException("La diferencia de niveles es mayor a 10");
        if (same_clan(attacker, target_player))
            throw AttackNotAllowedException("No puede haber ataques entre miembros del mismo clan");
    }

    DamageOutcome outcome = attacker->attack(*target, x, y);
    // El level-up puede ocurrir en CUALQUIER golpe (ganar_xp corre siempre, no
    // solo al matar). Lo capturamos acá para propagarlo aunque el target siga vivo.
    bool leveled_up = outcome.level_up;
    int level = leveled_up ? attacker->get_level() : 0;
    if (target->is_dead()) {
        if (outcome.gold_drop > 0)
            world.spawn_gold(x, y, static_cast<uint32_t>(outcome.gold_drop));
        if (target_is_player) {
            auto dropped = target_player->drop_inventory();
            world.scatter_items(x, y, std::move(dropped), players_in(z));
        } else if (outcome.dropped_item) {
            // Drop de la tabla de un NPC: poción o item al azar. Cae en la celda
            // del NPC; si está ocupada por otro item, se esparce a una libre.
            std::vector<std::unique_ptr<Item>> one;
            one.push_back(std::move(outcome.dropped_item));
            world.scatter_items(x, y, std::move(one), players_in(z));
        }
        return {true, true, target_is_player, target->get_name(), outcome.damage, x, y, outcome.dodged, leveled_up, level};
    }

    return {true, false, target_is_player, target->get_name(), outcome.damage, x, y, outcome.dodged, leveled_up, level};
}

void GameMap::self_cast(const std::string& player_name) {
    Player* player = find_player_by_name(player_name);
    if (player == nullptr) throw AttackerNotFoundException();
    player->cast_on_self();
}

bool GameMap::use_item(const std::string& player_name, uint64_t item_uid) {
    Player* player = find_player_by_name(player_name);
    if (player == nullptr) throw AttackerNotFoundException();
    return player->use_consumable(item_uid);
}

bool GameMap::update_npcs() {
    bool respawned = false;
    for (auto& [zone_id, world] : zones) {
        if (world.update_npcs()) respawned = true;
    }
    return respawned;
}

std::vector<NPCAttackEvent> GameMap::update_npc_aggro() {
    std::vector<NPCAttackEvent> all_events;

    for (auto& [zone_id, world] : zones) {
        auto players_in_zone = players_in(zone_id);
        auto zone_events = world.update_npc_aggro(players_in_zone);
        all_events.insert(all_events.end(), zone_events.begin(), zone_events.end());
    }
    return all_events;
}

std::vector<std::string> GameMap::tick(double seconds) {
    std::vector<std::string> meditating;
    for (auto& player : players) {
        if (player.tick(seconds)) {
            meditating.push_back(player.get_name());
        }
    }
    return meditating;
}

std::vector<std::string> GameMap::regen_all_players_life(double percent) {
    std::vector<std::string> regenerated;
    for (auto& player : players) {
        if (player.regen_life(percent)) {
            regenerated.push_back(player.get_name());
        }
    }
    return regenerated;
}

std::vector<std::string> GameMap::regen_all_players_mana(double percent) {
    std::vector<std::string> regenerated;
    for (auto& player : players) {
        if (player.regen_mana(percent)) {
            regenerated.push_back(player.get_name());
        }
    }
    return regenerated;
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

void GameMap::player_deposit_gold(const std::string& player_name, int amount) {
    Player* player = find_player_by_name(player_name);
    if (player == nullptr) throw std::runtime_error("Player not found.");
    NPCbanker* banker = zone_of(player_name).banker_adjacent_to(player->get_coord_x(), player->get_coord_y());
    if (banker == nullptr) throw std::runtime_error("No hay un banquero adyacente.");
    Command cmd;
    cmd.action = ACTION_DEPOSIT_GOLD;
    cmd.cantidad = amount;
    banker->interact(*player, cmd);
}

void GameMap::player_retire_item(const std::string& player_name, const std::string& item_id) {
    Player* player = find_player_by_name(player_name);
    if (player == nullptr) throw std::runtime_error("Player not found.");
    NPCbanker* banker = zone_of(player_name).banker_adjacent_to(player->get_coord_x(), player->get_coord_y());
    if (banker == nullptr) throw std::runtime_error("No hay un banquero adyacente.");
    Command cmd;
    cmd.action = ACTION_RETIRE;
    cmd.item_id = item_id;
    banker->interact(*player, cmd);
}

void GameMap::player_retire_gold(const std::string& player_name, int amount) {
    Player* player = find_player_by_name(player_name);
    if (player == nullptr) throw std::runtime_error("Player not found.");
    NPCbanker* banker = zone_of(player_name).banker_adjacent_to(player->get_coord_x(), player->get_coord_y());
    if (banker == nullptr) throw std::runtime_error("No hay un banquero adyacente.");
    Command cmd;
    cmd.action = ACTION_RETIRE_GOLD;
    cmd.cantidad = amount;
    banker->interact(*player, cmd);
}

std::vector<ItemInfo> GameMap::list_banker_items(const std::string& player_name) {
    Player* player = find_player_by_name(player_name);
    if (player == nullptr) throw std::runtime_error("Player not found.");
    NPCbanker* banker = zone_of(player_name).banker_adjacent_to(player->get_coord_x(), player->get_coord_y());
    if (banker == nullptr) throw std::runtime_error("No hay un banquero adyacente.");
    return banker->list_bank_items(player_name);
}

int GameMap::get_banker_gold(const std::string& player_name) {
    Player* player = find_player_by_name(player_name);
    if (player == nullptr) throw std::runtime_error("Player not found.");
    NPCbanker* banker = zone_of(player_name).banker_adjacent_to(player->get_coord_x(), player->get_coord_y());
    if (banker == nullptr) throw std::runtime_error("No hay un banquero adyacente.");
    return banker->get_bank_gold(player_name);
}

void GameMap::player_resurrect(const std::string& player_name) {
    Player* player = find_player_by_name(player_name);
    if (player == nullptr) throw std::runtime_error("Player not found.");
    NPCpriest* priest = zone_of(player_name).priest_adjacent_to(player->get_coord_x(), player->get_coord_y());
    if (priest == nullptr) throw std::runtime_error("No hay un sacerdote adyacente.");
    Command cmd;
    cmd.action = ACTION_RESURRECT;
    priest->interact(*player, cmd);
}
 
void GameMap::player_heal(const std::string& player_name) {
    Player* player = find_player_by_name(player_name);
    if (player == nullptr) throw std::runtime_error("Player not found.");
    NPCpriest* priest = zone_of(player_name).priest_adjacent_to(player->get_coord_x(), player->get_coord_y());
    if (priest == nullptr) throw std::runtime_error("No hay un sacerdote adyacente.");
    Command cmd;
    cmd.action = ACTION_HEAL;
    priest->interact(*player, cmd);
}
 
bool GameMap::player_buy_from_priest(const std::string& player_name, const std::string& item_id) {
    Player* player = find_player_by_name(player_name);
    if (player == nullptr) throw std::runtime_error("Player not found.");
    NPCpriest* priest = zone_of(player_name).priest_adjacent_to(player->get_coord_x(), player->get_coord_y());
    if (priest == nullptr) throw std::runtime_error("No hay un sacerdote adyacente.");
    Command cmd;
    cmd.action = ACTION_BUY;
    cmd.item_id = item_id;
    priest->interact(*player, cmd);
    return true;
}
 
std::vector<ItemInfo> GameMap::list_priest_items(const std::string& player_name) {
    Player* player = find_player_by_name(player_name);
    if (player == nullptr) throw std::runtime_error("Player not found.");
    int px = player->get_coord_x();
    int py = player->get_coord_y();
    NPCpriest* priest = zone_of(player_name).priest_adjacent_to(px, py);
    std::cout << "[DEBUG list_priest_items] player=(" << px << "," << py
              << ") priest=" << (priest ? "found" : "null") << std::endl;
    if (priest == nullptr) throw std::runtime_error("No hay un sacerdote adyacente.");
    return priest->list_items();
}

double GameMap::start_ghost_resurrect(const std::string& player_name) {
    Player* player = find_player_by_name(player_name);
    if (player == nullptr) throw std::runtime_error("Player not found.");
    if (!player->is_ghost()) throw std::runtime_error("El jugador no es un fantasma.");

    auto it = std::find_if(pending_resurrects.begin(), pending_resurrects.end(),
        [&player_name](const PendingRevive& pr) { return pr.player_name == player_name; });
    if (it != pending_resurrects.end())
        throw std::runtime_error("Ya estás en proceso de resucitar. Espera...");
 
    int px = player->get_coord_x();
    int py = player->get_coord_y();

    int best_dist_sq = INT_MAX;
    Zone best_zone = ZONE_DEFAULT;
    int best_priest_x = -1, best_priest_y = -1;
 
    for (auto& [zone_id, world] : zones) {
        auto [zx, zy] = world.nearest_priest_position(px, py);
        if (zx == -1) continue;
        int dx = zx - px;
        int dy = zy - py;
        int dist_sq = dx * dx + dy * dy;
        if (dist_sq < best_dist_sq) {
            best_dist_sq = dist_sq;
            best_zone = zone_id;
            best_priest_x = zx;
            best_priest_y = zy;
        }
    }
 
    if (best_dist_sq == INT_MAX)
        throw std::runtime_error("No hay ningún sacerdote en el mundo.");
 
    int dist = static_cast<int>(std::sqrt(static_cast<double>(best_dist_sq)));
    const GameConfig& cfg = GameConfig::instance();
    int ticks = std::max(1, static_cast<int>(dist * cfg.revive_ticks_per_distance));
 
    pending_resurrects.push_back({player_name, ticks, best_zone, best_priest_x, best_priest_y});
 
    return static_cast<double>(ticks) * cfg.tick_rate_ms / 1000.0;
}
 
std::vector<GameMap::ReviveResult> GameMap::tick_pending_resurrects() {
    std::vector<ReviveResult> completed;
 
    for (auto& pr : pending_resurrects) {
        pr.ticks_remaining--;
    }
 
    auto it = pending_resurrects.begin();
    while (it != pending_resurrects.end()) {
        if (it->ticks_remaining > 0) {
            ++it;
            continue;
        }
 
        const std::string name = it->player_name;
        Zone target_zone = it->target_zone;
        int priest_x = it->priest_x;
        int priest_y = it->priest_y;
        it = pending_resurrects.erase(it);
 
        Zone src_zone = zone_id_of(name);
        ZoneWorld& dst = zones.at(target_zone);
        auto [nx, ny] = dst.free_cell_adjacent_to(priest_x, priest_y, players_in(target_zone));
 
        std::cout << "[DEBUG: tick_pending_resurrects] " << name
                  << " priest=(" << priest_x << "," << priest_y << ")"
                  << " arrival=(" << nx << "," << ny << ")"
                  << " src_zone=" << static_cast<int>(src_zone)
                  << " target_zone=" << static_cast<int>(target_zone) << std::endl;
 
        TeleportResult tr;
        if (nx == -1) {
            // No hay celda libre junto al sacerdote: no se mueve de zona.
            tr = {false, ZONE_DESERT, 0, 0};
        } else {
            player_zone[name] = target_zone;
            Player* player = find_player_by_name(name);
            if (player != nullptr) player->update_position(nx, ny);
            tr = {true, target_zone, nx, ny, src_zone};
        }
 
        revive_player(name);
 
        completed.push_back({name, tr});
    }
 
    return completed;
}
 
void GameMap::cancel_ghost_resurrect(const std::string& player_name) {
    pending_resurrects.erase(
        std::remove_if(pending_resurrects.begin(), pending_resurrects.end(),
            [&player_name](const PendingRevive& pr) { return pr.player_name == player_name; }),
        pending_resurrects.end());
}
 
bool GameMap::is_resurrect_pending(const std::string& player_name) const {
    return std::any_of(pending_resurrects.begin(), pending_resurrects.end(),
        [&player_name](const PendingRevive& pr) { return pr.player_name == player_name; });
}


std::unique_ptr<Item> GameMap::pick_up_item(const std::string& player_name) {
    Player* player = find_player_by_name(player_name);
    if (!player) throw std::runtime_error("Player not found: " + player_name);
    if (!player->can_interact()) return nullptr;  // un fantasma no recolecta
    return zone_of(player_name).take_item_near(player->get_coord_x(), player->get_coord_y());
}

bool GameMap::pick_up_gold(const std::string& player_name) {
    Player* player = find_player_by_name(player_name);
    if (!player) throw std::runtime_error("Player not found: " + player_name);
    if (!player->can_interact()) return false;  // un fantasma no recolecta
    int amount = zone_of(player_name).take_gold_at(player->get_coord_x(), player->get_coord_y());
    if (amount > 0) {
        player->add_gold(amount);
        return true;
    }
    return false;
}

std::string GameMap::get_adjacent_npc_type(const std::string& player_name) {
    Player* player = find_player_by_name(player_name);
    if (player == nullptr) throw std::runtime_error("Player not found.");
    int px = player->get_coord_x();
    int py = player->get_coord_y();
    std::string type = zone_of(player_name).get_adjacent_friendly_type(px, py);
    std::cout << "[DEBUG get_adjacent_npc_type] player=" << player_name
              << " pos=(" << px << "," << py << ") npc_type=" << type << std::endl;
    return type;
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

void GameMap::cheat_lose_mana(const std::string& name, uint32_t amount) {
    Player* player = find_player_by_name(name);
    if (player == nullptr) return;
    player->lose_mana(static_cast<int>(amount));
}

uint32_t GameMap::get_player_xp(const std::string& name) {
    Player* player = find_player_by_name(name);
    return player->get_xp();
}

uint32_t GameMap::player_max_xp(const std::string& name) {
    Player* player = find_player_by_name(name);
    return player->max_xp();
}

uint32_t GameMap::get_player_mana(const std::string& name) {
    Player* player = find_player_by_name(name);
    return player->get_mana();
}

uint32_t GameMap::get_player_max_hp(const std::string& name) {
    Player* player = find_player_by_name(name);
    return player->get_max_life();
}

uint32_t GameMap::get_player_max_mana(const std::string& name) {
    Player* player = find_player_by_name(name);
    return player->get_max_mana();
}

std::vector<PlayerInfo> GameMap::build_players_snapshot(const std::string& player_name) {
    std::vector<PlayerInfo> snapshot;
    const Zone z = zone_id_of(player_name);
    for (const auto& p : players) {
        if (p.get_name() == player_name) continue;
        auto it = player_zone.find(p.get_name());
        if (it == player_zone.end() || it->second != z) continue;

        PlayerInfo pi{p.get_name(), p.get_race_name(), 0, p.get_coord_x(), p.get_coord_y()};
        pi.ghost = p.is_ghost();
        pi.equipped_ids = p.get_equipped_type_ids();
        pi.clan_name = clan_of(p.get_name());
        snapshot.push_back(pi);
    }
    return snapshot;
}

void GameMap::kill_player(const std::string& player_name) {
    ZoneWorld& world = zone_of(player_name);
    const Zone z = zone_id_of(player_name);
    Player* player = find_player_by_name(player_name);
    player->set_ghost();
    int x = player->get_coord_x();
    int y = player->get_coord_y();
    uint32_t gold_drop = player->gold_drop();
    if (gold_drop > 0)
        world.spawn_gold(x, y, gold_drop);
    auto dropped = player->drop_inventory();
    world.scatter_items(x, y, std::move(dropped), players_in(z));
}

void GameMap::load_clans(std::vector<Clan> persisted_clans) {
    clans.clear();
    for (Clan& clan : persisted_clans) {
        std::string name = clan.get_name();
        clans.emplace(std::move(name), std::move(clan));
    }
}

bool GameMap::player_already_in_a_clan(const std::string& player_name) {
    auto clan = std::find_if(clans.begin(), clans.end(), 
        [&player_name](auto& par) {
            // par.first es el nombre del clan (la clave)
            // par.second es el objeto Clan (el valor)
            return par.second.joined(player_name);
        }
    );

    if (clan == clans.end()) {
        return false;
    }
    return true;
}

bool GameMap::found_clan(const std::string& player_name, const std::string& clan_name) {
    if (player_already_in_a_clan(player_name)) {
        return false;
    }

    auto [it, created] = clans.try_emplace(clan_name, player_name, clan_name);
    return created;
}

bool GameMap::join_clan(const std::string& player_name, const std::string& clan_name) {
    if (player_already_in_a_clan(player_name)) {
        return false;
    }

    auto clan = clans.find(clan_name);
    if (clan == clans.end()) {
        return false;
    }

    Clan& wanted_clan = clan->second;
    if (!wanted_clan.join_request(player_name)) {
        return false;
    }
    return true;
}

std::string GameMap::rev_clan(const std::string& player_name) {
    auto clan = std::find_if(clans.begin(), clans.end(), 
        [&player_name](auto& par) {
            // par.first es el nombre del clan (la clave)
            // par.second es el objeto Clan (el valor)
            return par.second.is_founder(player_name);
        }
    );

    if (clan == clans.end()) {
        return "No eres fundador del clan " + clan->first;
    }

    Clan& wanted_clan = clan->second;
    std::string review = wanted_clan.review();
    return review;
}

void GameMap::accept_new_member(const std::string& player_name, const std::string& new_member) {
    auto clan = std::find_if(clans.begin(), clans.end(), 
        [&player_name](auto& par) {
            // par.first es el nombre del clan (la clave)
            // par.second es el objeto Clan (el valor)
            return par.second.is_founder(player_name);
        }
    );

    Clan& wanted_clan = clan->second;
    wanted_clan.accept_join_request(new_member);
}

bool GameMap::leave_clan(const std::string& player_name, std::string& clan_name) {
    auto clan = std::find_if(clans.begin(), clans.end(), 
        [&player_name](auto& par) {
            // par.first es el nombre del clan (la clave)
            // par.second es el objeto Clan (el valor)
            return par.second.joined(player_name);
        }
    );

    if (clan == clans.end()) {
        return false;
    }

    Clan& wanted_clan = clan->second;
    clan_name = wanted_clan.get_name();

    if (!wanted_clan.leave(player_name)) {
        return false;
    }
    return true;
}

bool GameMap::kick_member(const std::string& player_name, const std::string& member) {
    auto clan = std::find_if(clans.begin(), clans.end(), 
        [&player_name](auto& par) {
            // par.first es el nombre del clan (la clave)
            // par.second es el objeto Clan (el valor)
            return par.second.is_founder(player_name);
        }
    );

    Clan& wanted_clan = clan->second;
    if (!wanted_clan.kick(member)) {
        return false;
    }
    return true;
}

bool GameMap::ban_member(const std::string& player_name, const std::string& member) {
    auto clan = std::find_if(clans.begin(), clans.end(), 
        [&player_name](auto& par) {
            // par.first es el nombre del clan (la clave)
            // par.second es el objeto Clan (el valor)
            return par.second.is_founder(player_name);
        }
    );

    Clan& wanted_clan = clan->second;
    if (!wanted_clan.ban(member)) {
        return false;
    }
    return true;
}

TeleportResult GameMap::try_teleport_on_current_cell(const std::string& player_name) {
    ZoneWorld& zw = zone_of(player_name);
    Player* player = find_player_by_name(player_name);
    const TeleportDef* tp = zw.teleport_at(player->get_coord_x(), player->get_coord_y());
    if (tp == nullptr) {
        return {false, Zone::ZONE_DEFAULT,0,0};
    }
    auto it = ZONE_NAME_MAP.find(tp->dest_zone);
    if (it == ZONE_NAME_MAP.end()) {
        return {false, Zone::ZONE_DEFAULT,0,0};   // el string no es una zona conocida
    }
    Zone dest = it->second;
    return force_zone_change(player_name, dest);
}

void GameMap::set_infinite_hp(std::string player_name) {
    Player* player = find_player_by_name(player_name);
    player->heal_max_life();
    player->set_inmortal();
}
void GameMap::set_infinite_mana(std::string player_name) {
    Player* player = find_player_by_name(player_name);
    player->heal_max_mana();
    player->set_inf_mana();
}

void GameMap::revive_player(const std::string& player_name) {
    Player* player = find_player_by_name(player_name);
    if (player == nullptr) return;
    player->revive();
}
