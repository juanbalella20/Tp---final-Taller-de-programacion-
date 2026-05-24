#include "inventory.h"

bool Inventory::add_item(std::unique_ptr<Item> item) {
    if (is_full()) return false;
    items.push_back(std::move(item));
    return true;
}

std::unique_ptr<Item> Inventory::drop_item(Item* item) {
    // si el item que se tira estaba equipado, se desequipa primero
    if (equipped_item == item) {
        equipped_item = nullptr;
    }
    for (auto it = items.begin(); it != items.end(); ++it) {
        if (it->get() == item) {
            auto owned = std::move(*it);
            items.erase(it);
            return owned;
        }
    }
    return nullptr;
}

void Inventory::equip_item(Item* item) {
    // si había otro equipado lo desequipa
    if (equipped_item) {
        equipped_item = nullptr;
    }
    equipped_item = item;
}

void Inventory::unequip_item() {
    equipped_item = nullptr;
}

bool Inventory::is_full() const {
    return (int)items.size() >= MAX_SLOTS;
}

std::vector<Item*> Inventory::get_items() const {
    std::vector<Item*> result;
    result.reserve(items.size());
    for (const auto& item : items) {
        result.push_back(item.get());
    }
    return result;
}