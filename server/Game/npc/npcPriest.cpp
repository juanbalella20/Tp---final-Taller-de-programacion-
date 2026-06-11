#include "npcPriest.h"

#include "../player/player.h"
#include "game_config.h"
#include <stdexcept>
 
NPCpriest::NPCpriest(int x, int y) : pos_x(x), pos_y(y) {
    init_store();  
}

void NPCpriest::init_store() {
    // Stock del sacerdote segun config.toml ([priest].items): solo pociones y
    // hechizos. Via catalogo (unica fuente de nombre/precio/stats), cantidad
    // infinita. Los ids que no existan en el catalogo se ignoran.
    const auto& cfg = GameConfig::instance();
    for (const std::string& id : cfg.priest_items) {
        auto item = catalog.make_item(id);
        if (item) store_items.push_back(std::move(item));
    }
}

std::vector<ItemInfo> NPCpriest::list_items() const {
    std::vector<ItemInfo> result;
    for (const auto& item : store_items) {
        // uid = 0: catalogo de tipos (stock infinito), no instancias. El type
        // real permite a la ventana de comercio resolver el sprite/categoria.
        result.emplace_back(item->get_id(), item->getName(), item->getPrice(),
                            static_cast<uint8_t>(item->get_type()), 0);
    }
    return result;
}
 
void NPCpriest::resurrect(Player& player) {
    if (!player.is_ghost()) {
        throw std::runtime_error("No sos un fantasma, no necesitas resucitar.");
    }
    player.revive();
}
 
void NPCpriest::heal(Player& player) {
    if (player.is_ghost()) {
        throw std::runtime_error("Sos un fantasma, primero resucita.");
    }
    player.heal_max_life();
    player.heal_max_mana();
}
 
void NPCpriest::buy_item(Player& player, const std::string& item_id) {
    Item* store_item = nullptr;
    for (const auto& item : store_items) {
        if (item->get_id() == item_id) {
            store_item = item.get();
            break;
        }
    }
    if (store_item == nullptr)
        throw std::runtime_error("El sacerdote no vende ese item.");
 
    int price = store_item->getPrice();
    if (player.get_gold() < static_cast<uint32_t>(price))
        throw std::runtime_error("No tenes suficiente oro.");
 
    player.give_gold(price);
    auto new_item = catalog.make_item(item_id);
    if (new_item) player.add_item(std::move(new_item));
}
 
void NPCpriest::interact(Player& player, Command cmd) {
    if (cmd.action == ACTION_RESURRECT) {
        resurrect(player);
    } else if (cmd.action == ACTION_HEAL) {
        heal(player);
    } else if (cmd.action == ACTION_BUY) {
        buy_item(player, cmd.item_id);
    }
}
