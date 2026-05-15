#ifndef RANGED_WEAPON_H
#define RANGED_WEAPON_H

#include "weapon.h"

enum class RangedWeaponType { SIMPLE_BOW, COMPOUND_BOW };

class Player;

class RangedWeapon : public Weapon {
private:
    RangedWeaponType type;

public:
    explicit RangedWeapon(RangedWeaponType type);

    void attack(Player& target) override;
    void use_item(Player& player) override;

    RangedWeaponType getType() const;
};

#endif