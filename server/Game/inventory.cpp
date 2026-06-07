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

std::vector<std::unique_ptr<Item>> Inventory::drop_all() {
    equipped_item = nullptr;
    std::vector<std::unique_ptr<Item>> dropped = std::move(items);
    items.clear();
    return dropped;
}

void Inventory::equip_item(std::string item_id) {
    // El inventario solo equipa armas en su slot de arma. Los items de defensa
    // (armadura/casco/escudo) no se equipan acá — los maneja el DefenseSet.
    for (const auto& item : items) {
        if (item->get_id() == item_id) {
            // Solo armas o báculos ocupan este slot. Como es uno solo, equipar
            // un báculo desplaza al arma y viceversa (no se pueden tener ambos).
            if (item->get_type() != ItemType::WEAPON && item->get_type() != ItemType::MAGIC)
                return;
            equipped_item = item.get();
            return;
        }
    }
}

Item* Inventory::find_item(const std::string& item_id) const {
    for (const auto& item : items) {
        if (item->get_id() == item_id) return item.get();
    }
    return nullptr;
}

void Inventory::unequip_item() {
    equipped_item = nullptr;
}

bool Inventory::is_equipped(const std::string& item_id) const {
    return equipped_item != nullptr && equipped_item->get_id() == item_id;
}

bool Inventory::has_weapon_equipped() const {
    return equipped_item != nullptr;
}

std::string Inventory::get_equipped_weapon_id() const {
    return equipped_item != nullptr ? equipped_item->get_id() : std::string();
}

bool Inventory::is_full() const {
    return (int)items.size() >= MAX_SLOTS;
}

int Inventory::use_equipped(Entity& target, Player& atacante, int attacker_x, int attacker_y, int target_x, int target_y, bool is_critical) {
    if (!equipped_item)
        throw NoWeaponEquippedException();
    return equipped_item->use_item(target, atacante, attacker_x, attacker_y, target_x, target_y, is_critical);
}

std::vector<Item*> Inventory::get_items() const {
    std::vector<Item*> result;
    result.reserve(items.size());
    for (const auto& item : items) {
        result.push_back(item.get());
    }
    return result;
}