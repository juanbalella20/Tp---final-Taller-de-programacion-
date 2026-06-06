#include "npcSeller.h"
#include "../player/player.h"
#include "../item/item.h"
#include "../item/arma.h"
#include "game_config.h"

NPCseller::NPCseller(int x, int y) : pos_x(x), pos_y(y) {
    name = "Comerciante";
    init_store();
}
 
// TODO: cargar desde ItemDataBase cuando este implementada (pociones, armaduras, etc)
void NPCseller::init_store() {
    const auto& cfg = GameConfig::instance();
    store_items.push_back(std::make_unique<Arma>("espada",   cfg.espada.name,   cfg.espada.price,   cfg.espada.distance,   cfg.espada.damage_min,   cfg.espada.damage_max));
    store_items.push_back(std::make_unique<Arma>("hacha",    cfg.hacha.name,    cfg.hacha.price,    cfg.hacha.distance,    cfg.hacha.damage_min,    cfg.hacha.damage_max));
    store_items.push_back(std::make_unique<Arma>("martillo", cfg.martillo.name, cfg.martillo.price, cfg.martillo.distance, cfg.martillo.damage_min, cfg.martillo.damage_max));
}

int NPCseller::get_coord_x() const { return pos_x; }
int NPCseller::get_coord_y() const { return pos_y; }
 
std::vector<ItemInfo> NPCseller::list_items() const {
    std::vector<ItemInfo> result;
    for (const auto& item : store_items) {
        result.emplace_back(item->get_id(), item->getName(), item->getPrice());
    }
    return result;
}
 
void NPCseller::interact(Player& player, Command cmd) {
    if (cmd.action == ACTION_SELL) {
        // Player vende un item al seller
        Item* item_to_sell = nullptr;
        for (Item* item : player.get_inventory().get_items()) {
            if (item->get_id() == cmd.item_id) {
                item_to_sell = item;
                break;
            }
        }
        if (item_to_sell == nullptr) return;
 
        int price = item_to_sell->getPrice();
        player.drop_item(item_to_sell);
        player.add_gold(price);
 
    } else if (cmd.action == ACTION_BUY) {
        // Player compra un item del seller
        Item* store_item = nullptr;
        for (const auto& item : store_items) {
            if (item->get_id() == cmd.item_id) {
                store_item = item.get();
                break;
            }
        }
        if (store_item == nullptr) return;
 
        int price = store_item->getPrice();
        if (player.get_gold() < price) return;
 
        player.give_gold(price);
 
        // TODO: usar factory de items cuando este implementada
        auto new_item = std::make_unique<Arma>(
            store_item->get_id(),
            store_item->getName(),
            store_item->getPrice(),
            1, 2, 5
        );
        player.add_item(std::move(new_item));
    }
}

