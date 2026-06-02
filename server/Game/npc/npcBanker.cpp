#include "npcBanker.h"
#include <map>
#include <vector>
#include <iostream>
#include "inventory.h"
#include "../player/player.h"

NPCbanker::NPCbanker(int x, int y) : pos_x(x), pos_y(y) {}

int NPCbanker::get_coord_x() const { return pos_x; }

int NPCbanker::get_coord_y() const { return pos_y; }

void NPCbanker::deposit_item(Player& player, const std::string& item_id) {
    std::unique_ptr<Item> item = player.take_item_from_inventory(item_id);
    item_bank[player.get_name()].push_back(std::move(item));
}

void NPCbanker::deposit_gold(Player& player, int amount) {
    if (player.get_gold() < (uint32_t)amount)
        throw std::runtime_error("No tenes suficiente oro.");
    player.give_gold(amount);
    gold_bank[player.get_name()] += amount;
}

void NPCbanker::retire_item(Player& player, const std::string& item_id) {
    auto& items = item_bank[player.get_name()];
    for (auto it = items.begin(); it != items.end(); ++it) {
        if ((*it)->get_id() == item_id) {
            std::unique_ptr<Item> item = std::move(*it);
            items.erase(it);
            player.add_item(std::move(item));
            return;
        }
    }
    throw std::runtime_error("No tenes ese item en el banco.");
}

void NPCbanker::retire_gold(Player& player, int amount) {
    int& saved = gold_bank[player.get_name()];
    if (saved < amount)
        throw std::runtime_error("No tenes suficiente oro en el banco.");
    saved -= amount;
    player.add_gold(amount);
}

std::vector<ItemInfo> GameMap::list_banker_items(const std::string& player_name) {
    Player* player = find_player_by_name(player_name);
    if (player == nullptr) throw std::runtime_error("Player not found.");
    NPCbanker* banker = zone_of(player_name).banker_adjacent_to(player->get_coord_x(), player->get_coord_y());
    if (banker == nullptr) throw std::runtime_error("No hay un banquero adyacente.");
    return banker->list_bank(player_name);
}

void NPCbanker::interact(Player& player, Command cmd) {
    switch (cmd.action) {
        case ACTION_DEPOSIT:
            deposit_item(player, cmd.item_id);
            break;
        case ACTION_DEPOSIT_GOLD:
            deposit_gold(player, cmd.cantidad);
            break;
        case ACTION_RETIRE:
            retire_item(player, cmd.item_id);
            break;
        case ACTION_RETIRE_GOLD:
            retire_gold(player, cmd.cantidad);
            break;
        default:
            break;
    }
}
