#include "inventory.h"
#include "../game_exceptions.h"

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

void Inventory::equip_item(std::string item_id) {
    // si había otro equipado lo desequipa
    if (equipped_item) {
        equipped_item = nullptr;
    }
    for (const auto& item : items) {
        if (item->get_id() == item_id) {
            equipped_item = item.get();
            break;
        }
    }
}

void Inventory::unequip_item() {
    equipped_item = nullptr;
}

bool Inventory::is_full() const {
    return (int)items.size() >= MAX_SLOTS;
}

int Inventory::use_equipped(Entity& target, Player& atacante, int attacker_x, int attacker_y, int target_x, int target_y) {
    if (!equipped_item)
        throw NoWeaponEquippedException();
    return equipped_item->use_item(target, atacante, attacker_x, attacker_y, target_x, target_y);
}

std::vector<Item*> Inventory::get_items() const {
    std::vector<Item*> result;
    result.reserve(items.size());
    for (const auto& item : items) {
        result.push_back(item.get());
    }
    return result;
}