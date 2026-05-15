#include "melee_weapon.h"
#include "player.h"

//No deberian ir hardcodeados pero despues se cambiara para integrar el TOML
MeleeWeapon::MeleeWeapon(MeleeWeaponType type)
    : Weapon(0, "", 0, 0, 0), type(type) {
    switch (type) {
        case MeleeWeaponType::SWORD:
            id = 1; name = "Espada"; price = 50;
            damage_min = 2; damage_max = 5;
            break;
        case MeleeWeaponType::AXE:
            id = 2; name = "Hacha"; price = 60;
            damage_min = 4; damage_max = 5;
            break;
        case MeleeWeaponType::HAMMER:
            id = 3; name = "Martillo"; price = 55;
            damage_min = 1; damage_max = 9;
            break;
    }
}

void MeleeWeapon::attack(Player& target) {
    // el calculo de daño real va en el servidor con la formula de la spec
    // Daño = Fuerza * rand(DañoMin, DañoMax)
}

void MeleeWeapon::use_item(Player& player) {
    player.equip_item(getId());
}

MeleeWeaponType MeleeWeapon::getType() const { 
    return type; 
}