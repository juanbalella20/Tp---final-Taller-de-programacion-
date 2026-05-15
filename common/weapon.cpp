#include "weapon.h"

Weapon::Weapon(int id, const std::string& name, int price, int damage_min, int damage_max)
    : Item(id, name, price) , damage_min(damage_min), damage_max(damage_max) {}

int Weapon::getDamageMin() const { 
    return damage_min; 
}

int Weapon::getDamageMax() const { 
    return damage_max; 
}