#include "npcSeller.h"
#include "player.h"
#include "item.h"
#include "arma.h"

NPCseller::NPCseller(int x, int y) : pos_x(x), pos_y(y) {
    name = "Comerciante";
    init_store();
}
 
// TODO: cargar desde ItemDataBase cuando este implementada (pociones, armaduras, etc)
void NPCseller::init_store() {
    store_items.push_back(std::make_unique<Arma>("espada",   "Espada",   100, 1, 5));
    store_items.push_back(std::make_unique<Arma>("hacha",    "Hacha",    150, 1, 7));
    store_items.push_back(std::make_unique<Arma>("martillo", "Martillo", 120, 1, 6));
}
 
std::vector<ItemInfo> NPCseller::list_items() const {
    std::vector<ItemInfo> result;
    for (const auto& item : store_items) {
        result.emplace_back(item->get_id(), item->getName(), item->getPrice());
    }
    return result;
}
 
void NPCseller::interact(Player& player, Command cmd) {
    // Buscamos el item en el inventario del player
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
}

