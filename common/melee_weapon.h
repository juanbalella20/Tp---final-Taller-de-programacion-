#ifndef MELEE_WEAPON_H
#define MELEE_WEAPON_H

#include "weapon.h"

enum class MeleeWeaponType { SWORD, AXE, HAMMER };

class Player;

class MeleeWeapon : public Weapon {
private:
    MeleeWeaponType type;

public:
    explicit MeleeWeapon(MeleeWeaponType type);

    void attack(Player& target) override;
    void use_item(Player& player) override;

    MeleeWeaponType getType() const;
};

#endif