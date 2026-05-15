#ifndef WEAPON_H
#define WEAPON_H

#include "item.h"

class Player;

class Weapon : public Item {
protected:
    int damage_min;
    int damage_max;

public:
    Weapon(int id, const std::string& name, int price, int damage_min, int damage_max);
    virtual ~Weapon() = default;

    virtual void attack(Player& target) = 0;

    int getDamageMin() const;
    int getDamageMax() const;
};

#endif