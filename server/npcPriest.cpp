#include "npcPriest.h"


NPCpriest::NPCpriest() { 
    set_npc_name("Priest");
}

Store NPCpriest::get_store() { 
    return store.get_store();
}

void NPCpriest::update_store(Item item) { 
    store.delete_item(Item item);
}

void NPCpriest::revive(Player player) { 
    /* no hace falta esto: 
    Event event;
    event.type = REVIVE;
    event.player_id = player_tag;
    queue_event.push(event);
    */
    player.revive();
}

void NPCpriest::heal(Player player) {
    player.heal();
}

void NPCpriest::sell_item(Item item, Player player, int cantidad) {
    if (player.get_gold() >= item.getprice*cantidad) {
        player.add_item(Item item);
        update_store(Item item);
    }
    else {
    // send "not enough money" message to player
    }
}

void NPCpriest::interact(Player player, Command cmd) { 
    switch (cmd.type) {
        case HEAL:
            heal(player);
            break;
        case SELL:
            sell_item(cmd.objeto, player, cmd.cantidad);
            break;
        case REVIVE :
            revive(player);
            break;
        default:
            break;
    }
}
