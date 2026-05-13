#include "npcPriest.h"

NPCpriest::NPCpriest() { 
    // todo
}

Store NPCpriest::get_store() { 
    return NPCpriest::store.get_store();
}

void NPCpriest::update_store(Item item) { 
    store.delete_item(Item item);
}

void NPCpriest::revive(int player_tag) { 
    Event event;
    event.type = REVIVE;
    event.player_id = player_tag;
    queue_event.push(event) 
    /* de la misma manera que player ineractua
    con game, priest deberia interactuar con game*/
}

void NPCpriest::heal(int player_tag) { 
    Event event;
    event.type = HEAL;
    event.player_id = player_tag;
    queue_event.push(event);
    /* de la misma manera que player ineractua
    con game, priest deberia hacer lo mismo*/
}

Item NPCpriest::sell_item(int item_id, int player_tag, int gold, int cantidad) { 
    Item item = store.find_item(item_id);
    if (gold) >= item.getprice*cantidad {
        Event event;
        event.type = GIVE_ITEM;
        event.player_id = player_tag;
        queue_event.push(event)
        update_store(Item item);
    }
    else 
        // send "not enough money" message to player
        // ver: que deberia devolver?

}

void NPCpriest::interact(int player_tag, Command cmd) { 
    switch (cmd.type) {
        case HEAL:
            heal(player_tag);
            break;
        case SELL():
            sell_item(cmd.objeto, player_tag, cmd.acutal_gold, cmd.cantidad);
            break;
        case REVIVE :
            revive(player_tag);
            break;
        default:
            break;
    }
}
