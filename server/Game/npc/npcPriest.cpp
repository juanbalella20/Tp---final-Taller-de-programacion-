#include "npcPriest.h"

#include "../player/player.h"
#include "game_config.h"
#include <stdexcept>
 
void NPCpriest::init_store() {
    // El sacerdote vende baculos, varas y pociones (nunca armas ni armaduras)
    for (const char* id : {"vara_fresno", "flauta_elfica", "baculo_nudoso",
                           "baculo_engarzado", "pocion_vida", "pocion_mana"}) {
        auto item = catalog.make_item(id);
        if (item) store_items.push_back(std::move(item));
    }
}
 
std::vector<ItemInfo> NPCpriest::list_items() const {
    std::vector<ItemInfo> result;
    for (const auto& item : store_items) {
        result.emplace_back(item->get_id(), item->getName(), item->getPrice());
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
 
void NPCpriest::sell_item(Player& player, const std::string& item_id) {
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
    } else if (cmd.action == ACTION_SELL) {
        sell_item(player, cmd.item_id);
    }
}
