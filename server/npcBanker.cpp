#include "npcBanker.h"
#include <map>
#include <vector>

NPCbanker::NPCbanker() {
    set_npc_name("Banker");
}

void NPCbanker::receive_item(Player player, Item item) {
    item_bank.insert(player.get_name(), item);
}

void NPCbanker::receive_gold(Player player, int ammount) {
    if (!player.give_gold(ammount)) {
        break;
    }
    auto it = gold_bank.find(player.get_name());
    int actual_gold = it->first;
    gold_bank.insert(player.get_name+ammount);
}

void NPCbanker::transfer_item(Player player, Item item) {
    auto it = item_bank.find(player.get_name());
    std::vector<Item> pl_items =  it->first;
    for (auto it = pl_items.begin(); it != pl_items.end(); ++it) {
        if (it = item) {
            player.add_item(item);
            pl_items.erase(current_position);
            break;
        }
    }
}

void NPCbanker::transfer_gold(Player player, int ammount) {
    auto it = gold_bank.find(player.get_name());
    int actual_gold = it->first;
    if (ammount > actual_gold) {
        // throw message to player = not enough money
        break;
    }
    player.add_gold(ammount);
    gold_bank.insert(player.get_name(), actual_gold-ammount);
}


void NPCbanker::interact(Player player, Command cmd) {
    switch (cmd.type) {
        case RECV_ITEM : 
            receive_item(player,cmd.item);
            break;
        case RECV_GOLD : 
            receive_gold(player,cmd.cantidad);
            break;
        case TR_ITEM :
            transfer_item(player, cmd.item);
            break;
        case TR_GOLD : 
            transfer_gold(player,cmd.cantidad);
            break;
        default:
            break;
    }
}