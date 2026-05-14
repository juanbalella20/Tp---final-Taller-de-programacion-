#include "inventory.h"

void Inventory::add_item(Item item) {
    items.push_back(item);
}

void Inventory::drop_item(Item item) {
    std::erase_if(items, [&item](const Item& item_actual) {
        return item_actual.id == item.id;
    });
}

void Inventory::equip_item(Item item) {
    drop_item(item);
}

void Inventory::unequip_item(Item item) {
    add_item(item);
}
