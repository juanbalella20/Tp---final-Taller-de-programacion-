#include "zoneWorld.h"

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <stdexcept>

#include "../../common/mapLoader.h"
#include "item/item.h"
#include "player/player.h"

void ZoneWorld::load_terrain(const std::string& toml_path) {
    MapLoader md;
    md.load(toml_path);

    width = md.get_width();
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
    // [[teleport]] = tiles de teletransporte hacia otra zona.
    teleports = md.get_teleports();
    for (const auto& [name, pos] : spawns) {
        if (name == "seller") spawn_seller(pos.x, pos.y);
        else if (name == "banker") spawn_banker(pos.x, pos.y);
        else if (name == "priest") spawn_priest(pos.x, pos.y);
    }

}

bool ZoneWorld::in_bounds(int x, int y) const {
    return x >= 0 && x < width && y >= 0 && y < height;
}

bool ZoneWorld::is_blocked_terrain(int x, int y) const {
    if (!in_bounds(x, y)) return true;
    return map[y][x] != elements::empty;
}

bool ZoneWorld::has_actor_at(int x, int y,
                             const std::vector<const Player*>& players_here) const {
    for (const Player* p : players_here) {
        if (p->get_coord_x() == x && p->get_coord_y() == y) return true;
    }
    for (const auto& n : npcs) {
        if (!n.is_dead() && n.get_coord_x() == x && n.get_coord_y() == y) return true;
    }
    return false;
}

bool ZoneWorld::has_ground_item_at(int x, int y) const {
    for (const auto& gi : ground_items) {
        if (gi.pos.x == x && gi.pos.y == y) return true;
    }
    return false;
}

std::pair<int, int> ZoneWorld::find_random_empty_cell(
    const std::vector<const Player*>& players_here) const {
    std::vector<std::pair<int, int>> empty_cells;
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            if (map[y][x] == elements::empty && !has_actor_at(x, y, players_here)) {
                empty_cells.emplace_back(x, y);
            }
        }
    }
    if (empty_cells.empty()) return {-1, -1};
    return empty_cells[rand() % empty_cells.size()];  // mejorar random
}

void ZoneWorld::spawn_npc(NPChostile&& npc) {
    int x = npc.get_coord_x();
    int y = npc.get_coord_y();
    if (in_bounds(x, y)) {
        npcs.push_back(std::move(npc));
        std::cout << "[DEBUG: spawn_npc] " << npcs.back().get_name()
                  << " at (" << x << "," << y << ")" << std::endl;
    }
}

void ZoneWorld::spawn_seller(int x, int y) {
    if (in_bounds(x, y)) {
        sellers.emplace_back(x, y);
        map[y][x] = elements::npcs;
        std::cout << "[DEBUG: spawn_seller] NPCseller at (" << x << "," << y << ")" << std::endl;
    }
}

void ZoneWorld::spawn_item(int x, int y, std::unique_ptr<Item> item) {
    if (in_bounds(x, y)) {
        std::string id = item->get_id();
        positionCoord coord{x, y};
        ground_items.push_back({coord, std::move(item)});
        std::cout << "[DEBUG: spawn_item] " << id << " at (" << x << "," << y << ")" << std::endl;
    }
}

void ZoneWorld::spawn_gold(int x, int y, int amount) {
    if (in_bounds(x, y)) {
        positionCoord coord{x, y};
        ground_gold.push_back({coord, amount});
        std::cout << "[DEBUG: spawn_gold] at (" << x << "," << y << ")" << std::endl;
    }
}

void ZoneWorld::spawn_banker(int x, int y) {
    bankers.emplace_back(x, y);
}

void ZoneWorld::spawn_priest(int x, int y) {
    priests.emplace_back(x, y);
}

bool ZoneWorld::update_npcs() {
    bool respawned = false;
    for (auto& npc : npcs) {
        if (!npc.is_dead()) continue;
        npc.reduce_ticks_to_spawn();
        if (!npc.can_spawn()) continue;

        auto [rx, ry] = find_random_empty_cell({});
        if (rx == -1) continue;

        npc.revive(rx, ry);
        std::cout << "[DEBUG: update_npcs] " << npc.get_name()
                  << " respawned at (" << rx << "," << ry << ")" << std::endl;
        respawned = true;
    }
    return respawned;
}

std::vector<NpcInfo> ZoneWorld::build_npcs_snapshot() const {
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
    for (const auto& s : sellers) {
        NpcInfo npcinfo;
        npcinfo.name = "Comerciante";
        npcinfo.type = "seller";
        npcinfo.x = s.get_coord_x();
        npcinfo.y = s.get_coord_y();
        snapshot.push_back(npcinfo);
    }
    for (const auto& b : bankers) {
        NpcInfo npcinfo;
        npcinfo.name = "Banquero";
        npcinfo.type = "banker";
        npcinfo.x = b.get_coord_x();
        npcinfo.y = b.get_coord_y();
        snapshot.push_back(npcinfo);
    }
    for (const auto& p : priests) {
        NpcInfo npcinfo;
        npcinfo.name = "Sacerdote";
        npcinfo.type = "priest";
        npcinfo.x = p.get_coord_x();
        npcinfo.y = p.get_coord_y();
        snapshot.push_back(npcinfo);
    }

    return snapshot;
}

std::vector<ItemFloorInfo> ZoneWorld::build_items_snapshot() const {
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

static bool is_adyacent(const positionCoord& a, const positionCoord& b) {
    return std::abs(a.x - b.x) + std::abs(a.y - b.y) <= 1;
}

NPCseller* ZoneWorld::seller_adjacent_to(int px, int py) {
    positionCoord player_pos{px, py};
    for (auto& s : sellers) {
        positionCoord seller_pos{s.get_coord_x(), s.get_coord_y()};
        if (is_adyacent(player_pos, seller_pos)) return &s;
    }
    return nullptr;
}

NPCseller* ZoneWorld::seller_at(int x, int y) {
    positionCoord player_pos{x, y};
    for (auto& s : sellers) {
        positionCoord seller_pos{s.get_coord_x(), s.get_coord_y()};
        if (is_adyacent(player_pos, seller_pos)) return &s;
    }
    return nullptr;
}

std::vector<ItemInfo> ZoneWorld::list_seller_items(int px, int py) {
    NPCseller* seller = seller_adjacent_to(px, py);
    if (seller == nullptr) throw std::runtime_error("No hay un comerciante adyacente.");
    return seller->list_items();
}

std::unique_ptr<Item> ZoneWorld::take_item_near(int px, int py) {
    positionCoord player_pos{px, py};
    // Items: cualquiera adyacente
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

int ZoneWorld::take_gold_at(int px, int py) {
    // Oro: solo si esta exactamente debajo del player
    auto gold_it = std::find_if(ground_gold.begin(), ground_gold.end(),
        [px, py](const groundGold& g_gold) {
            return g_gold.pos.x == px && g_gold.pos.y == py;
        });

    if (gold_it != ground_gold.end()) {
        int amount = gold_it->amount;
        ground_gold.erase(gold_it);
        return amount;
    }
    return 0;
}

NPChostile* ZoneWorld::hostile_at(int x, int y) {
    for (auto& npc : npcs) {
        if (!npc.is_dead() && npc.get_coord_x() == x && npc.get_coord_y() == y) {
            return &npc;
        }
    }
    return nullptr;
}

const TeleportDef* ZoneWorld::teleport_adjacent_to(int x, int y) const {
    positionCoord here{x, y};
    for (const auto& tp : teleports) {
        if (is_adyacent(positionCoord{tp.x, tp.y}, here)) return &tp;
    }
    return nullptr;
}

std::pair<int, int> ZoneWorld::free_cell_adjacent_to(
    int tx, int ty, const std::vector<const Player*>& players_here) const {
    // Las 4 celdas adyacentes (N, S, E, O) al teleport destino.
    const int dx[] = {0, 0, 1, -1};
    const int dy[] = {-1, 1, 0, 0};
    for (int i = 0; i < 4; ++i) {
        int nx = tx + dx[i];
        int ny = ty + dy[i];
        if (in_bounds(nx, ny) && map[ny][nx] == elements::empty &&
            !has_actor_at(nx, ny, players_here) && !has_ground_item_at(nx, ny)) {
            return {nx, ny};
        }
    }
    // Fallback: cualquier celda libre de la zona.
    return find_random_empty_cell(players_here);
}
