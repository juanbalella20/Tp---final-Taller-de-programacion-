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
