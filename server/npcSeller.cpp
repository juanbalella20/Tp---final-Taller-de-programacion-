#include "npcSeller.h"

NPCseller::NPCseller() {
    set_npc_name("Seller");
}

Store NPCseller::get_store() {
    // solo debe devoler los items con stock > 1
    return store.get_store();
}

void NPCseller::add_item_store(Item item) {
    store.add_item();
}

void NPCseller::delete_item_store(Item item) {
    store.delete_item(Item item);
}

Item NPCseller::sell_item(Item item, Player player) {
    if (player.get_gold() >= item.getprice*cantidad) {
        player.add_item(Item item);
        delete_item_store(Item item);
    }
    else {
    // send "not enough money" message to player
    }
}

void NPCseller::buy_item(Item item, Player player) {
    player.add_gold(item.price);
    add_item_store(item);
}

void NPCseller::interact(Player player, Command cmd) {
    switch (cmd.type) {
        case SELL :
            sell_item(cmd.item, player);
            break;
        case BUY :
            buy_item(cmd.item, player);
            break;
        case GET_STORE :
            get_store();
            break;
        default:
            break;
    }
}