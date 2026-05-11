#ifndef INVENTORY_H_
#define INVENTORY_H_

#include <vector>

class Inventory {

private:
    std::vector<Item> items;

public:
    void equip(Item item);

    void unequip(Item item);
};


#endif
