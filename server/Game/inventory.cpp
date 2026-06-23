#include "inventory.h"
#include "../game_exceptions.h"
#include "player/player.h"
#include "../../common/constants/game_config.h"

bool Inventory::add_item(std::unique_ptr<Item> item) {
    if (is_full()) return false;
    items.push_back(std::move(item));
    return true;
}

std::unique_ptr<Item> Inventory::drop_item(Item* item) {
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

std::vector<std::unique_ptr<Item>> Inventory::drop_all() {
    equipped_item = nullptr;
    std::vector<std::unique_ptr<Item>> dropped = std::move(items);
    items.clear();
    return dropped;
}

void Inventory::equip_item(uint64_t item_uid) {
    for (const auto& item : items) {
        if (item->get_uid() == item_uid) {
            if (item->get_type() != ItemType::WEAPON && item->get_type() != ItemType::MAGIC)
                return;
            equipped_item = item.get();
            return;
        }
    }
}

Item* Inventory::find_item(uint64_t item_uid) const {
    for (const auto& item : items) {
        if (item->get_uid() == item_uid) return item.get();
    }
    return nullptr;
}

void Inventory::unequip_item() {
    equipped_item = nullptr;
}

bool Inventory::is_equipped(uint64_t item_uid) const {
    return equipped_item != nullptr && equipped_item->get_uid() == item_uid;
}

bool Inventory::has_weapon_equipped() const {
    return equipped_item != nullptr;
}

uint64_t Inventory::get_equipped_weapon_uid() const {
    return equipped_item != nullptr ? equipped_item->get_uid() : 0;
}

std::string Inventory::get_equipped_weapon_type_id() const {
    return equipped_item != nullptr ? equipped_item->get_id() : std::string();
}

bool Inventory::is_full() const {
    return (int)items.size() >= GameConfig::instance().max_inventory_slots;
}

DamageOutcome Inventory::use_equipped(Entity& target, Player& atacante, int attacker_x, int attacker_y, int target_x, int target_y, bool is_critical) {
    if (!equipped_item)
        throw NoWeaponEquippedException();
    if (&target == static_cast<Entity*>(&atacante) && !equipped_item->can_target_self())
        return{0,0,false,false,nullptr};
    return equipped_item->use_item(target, atacante, attacker_x, attacker_y, target_x, target_y, is_critical);
}

bool Inventory::use_consumable(uint64_t item_uid, Player& self) {
    for (auto it = items.begin(); it != items.end(); ++it) {
        if ((*it)->get_uid() != item_uid) continue;

        if ((*it)->get_type() != ItemType::OTHER) return false;

        (*it)->use_item(self, self, 0, 0, 0, 0, false);
        items.erase(it);
        return true;
    }
    return false;
}

std::vector<Item*> Inventory::get_items() const {
    std::vector<Item*> result;
    result.reserve(items.size());
    for (const auto& item : items) {
        result.push_back(item.get());
    }
    return result;
}

Item* Inventory::find_by_id(const std::string& item_id) const {
    for (const auto& item : items) {
        if (item->get_id() == item_id) return item.get();
    }
    return nullptr;
}
