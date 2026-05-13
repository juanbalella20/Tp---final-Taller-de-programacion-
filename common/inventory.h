#ifndef INVENTORY_H_
#define INVENTORY_H_

#include <vector>

class Inventory {

private:
    std::vector<Item> items;

public:
    void add_item(Item item);

    void drop_item(Item item);

    void equip_item(Item item);

    void unequip_item(Item item);
};


#endif
