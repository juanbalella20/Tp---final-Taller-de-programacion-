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
    player.give_gold(-amount);
    gold_bank[player.get_name()] += amount;
}

void NPCbanker::interact(Player& player, Command cmd) {
    switch (cmd.action) {
        case ACTION_DEPOSIT:
            deposit_item(player, cmd.item_id);
            break;
        case ACTION_DEPOSIT_GOLD:
            deposit_gold(player, cmd.cantidad);
            break;
        default:
            break;
    }
}
