#ifndef INVENTORY_H_
#define INVENTORY_H_

#include <vector>

#include "item.h"

class Inventory {

private:
    std::vector<Item> items;

public:
    Inventory() {}

    void add_item(Item item);

    void drop_item(Item item);

    void equip_item(Item item);

    void unequip_item(Item item);
};


#endif