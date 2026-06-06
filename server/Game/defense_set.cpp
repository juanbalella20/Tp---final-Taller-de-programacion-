#include "defense_set.h"

void DefenseSet::equip_armadura(DefenseItem* item) {
    armadura = item;
}

void DefenseSet::equip_casco(DefenseItem* item) {
    casco = item;
}

void DefenseSet::equip_escudo(DefenseItem* item) {
    escudo = item;
}

void DefenseSet::unequip_armadura() {
    armadura = nullptr;
}

void DefenseSet::unequip_casco() {
    casco = nullptr;
}

void DefenseSet::unequip_escudo() {
    escudo = nullptr;
}

bool DefenseSet::is_equipped(const std::string& item_id) const {
    if (armadura && armadura->get_id() == item_id) return true;
    if (casco    && casco->get_id() == item_id)    return true;
    if (escudo   && escudo->get_id() == item_id)   return true;
    return false;
}

std::vector<std::string> DefenseSet::get_equipped_ids() const {
    std::vector<std::string> ids;
    if (armadura) ids.push_back(armadura->get_id());
    if (casco)    ids.push_back(casco->get_id());
    if (escudo)   ids.push_back(escudo->get_id());
    return ids;
}

int DefenseSet::calculate_defense() const {
    int total = 0;
    if (armadura) total += armadura->calculate_defense();
    if (casco)    total += casco->calculate_defense();
    if (escudo)   total += escudo->calculate_defense();
    return total;
}

void DefenseSet::clear() {
    armadura = nullptr;
    casco = nullptr;
    escudo = nullptr;
}
