#ifndef INVENTORY_H_
#define INVENTORY_H_

#include <vector>
#include <memory>

#include "item.h"

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

    void equip_item(Item* item);

    void unequip_item();

    bool is_full() const;
};


#endif