#include "ranged_weapon.h"
#include "player.h"

//Provisorio hasta integrar TOML
RangedWeapon::RangedWeapon(RangedWeaponType type)
    : Weapon(0, "", 0, 0, 0), type(type) {
    switch (type) {
        case RangedWeaponType::SIMPLE_BOW:
            id = 4; name = "Arco simple"; price = 40;
            damage_min = 1; damage_max = 4;
            break;
        case RangedWeaponType::COMPOUND_BOW:
            id = 5; name = "Arco compuesto"; price = 120;
            damage_min = 4; damage_max = 16;
            break;
    }
}

void RangedWeapon::attack(Player& target) {
    // ataque a distancia misma formula, sin requerir adyacencia
    // Daño = Fuerza * rand(DañoArmaMin, DañoArmaMax)
}

void RangedWeapon::use_item(Player& player) {
    player.equip_item(getId());
}

RangedWeaponType RangedWeapon::getType() const { 
    return type; 
}