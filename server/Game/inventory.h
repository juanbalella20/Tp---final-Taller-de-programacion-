#ifndef INVENTORY_H_
#define INVENTORY_H_

#include <vector>
#include <memory>

#include "item/item.h"
#include "entity.h"

class Player;

class Inventory {

private:
    std::vector<std::unique_ptr<Item>> items;
    Item* equipped_item = nullptr;

    static constexpr int MAX_SLOTS = 25;  //a definir

public:
    Inventory() {}

    std::vector<Item*> get_items() const;

    bool add_item(std::unique_ptr<Item> item);

    std::unique_ptr<Item> drop_item(Item* item);

    // Devuelve todos los items del inventario y lo deja vacío.
    std::vector<std::unique_ptr<Item>> drop_all();

    void equip_item(std::string item_id);

    // Devuelve el item con ese id (sin ceder ownership), o nullptr si no está.
    Item* find_item(const std::string& item_id) const;

    void unequip_item();

    bool is_full() const;
    int use_equipped(Entity& target, Player& atacante, int attacker_x, int attacker_y, int target_x, int target_y, bool is_critical);
};


#endif