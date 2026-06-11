#include "npcSeller.h"
#include "../player/player.h"
#include "../item/item.h"
#include "../item/item_catalog.h"
#include "game_config.h"
#include <stdexcept>

NPCseller::NPCseller(int x, int y) : pos_x(x), pos_y(y) {
    name = "Comerciante";
    init_store();
}

// Stock del comerciante segun config.toml ([seller].items), via catalogo
// (unica fuente de verdad de nombre/precio/stats). La cantidad de cada item es
// infinita: el stock es un catalogo de tipos, no un set de copias que se agote.
// Los ids que no existan en el catalogo se ignoran.
void NPCseller::init_store() {
    const auto& cfg = GameConfig::instance();
    for (const std::string& id : cfg.seller_items) {
        auto item = catalog.make_item(id);
        if (item != nullptr) store_items.push_back(std::move(item));
    }
}

int NPCseller::get_coord_x() const { return pos_x; }
int NPCseller::get_coord_y() const { return pos_y; }
 
std::vector<ItemInfo> NPCseller::list_items() const {
    std::vector<ItemInfo> result;
    for (const auto& item : store_items) {
        // uid = 0: es un catalogo de tipos (stock infinito), no instancias.
        result.emplace_back(item->get_id(), item->getName(), item->getPrice(),
                            static_cast<uint8_t>(item->get_type()), 0);
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
        if (player.get_gold() < price) throw std::runtime_error("No tenes suficiente oro.");

        // El inventario tiene un tope (config.toml: player.max_inventory_slots).
        // Verificamos ANTES de cobrar: si esta lleno, add_item descartaria el
        // item en silencio y el jugador perderia el oro sin recibir nada.
        if (player.get_inventory().is_full())
            throw std::runtime_error("Inventario lleno.");

        // Reconstruye el item con su subtipo y stats reales via catalogo
        // (antes se creaba siempre como Arma con stats inventados -> bug).
        auto new_item = catalog.make_item(store_item->get_id());
        if (new_item == nullptr) return;

        player.give_gold(price);
        player.add_item(std::move(new_item));
    }
}

