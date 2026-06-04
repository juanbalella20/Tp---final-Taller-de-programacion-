#include "defense_item.h"
#include <cstdlib>

DefenseItem::DefenseItem(const std::string& id, const std::string& name, int price,
                         int defense_min, int defense_max):
        Item(id, name, price), defense_min(defense_min), defense_max(defense_max) {}

int DefenseItem::calculate_defense() const {
    int rango = defense_max - defense_min;
    int roll = (rango > 0) ? (std::rand() % (rango + 1)) : 0;
    return defense_min + roll;
}

int DefenseItem::use_item(Entity& target, Player& atacante,
                          int attacker_x, int attacker_y,
                          int target_x, int target_y, bool is_critical) {
    // "Usar" un item de defensa = la defensa que aporta. Ignora los parámetros
    // de ataque.
    (void)target; (void)atacante;
    (void)attacker_x; (void)attacker_y;
    (void)target_x; (void)target_y; (void)is_critical;
    return calculate_defense();
}
