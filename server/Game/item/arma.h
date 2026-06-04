#ifndef ARMA_H
#define ARMA_H

#include <string>

#include "item.h"
#include "../entity.h"

class Arma: public Item {
private:
    int disntance_min_attack;
    int damage_min;
    int damage_max;

public:
    Arma(const std::string& id, const std::string& name, int price, int disntance_min_attack,
         int damage_min, int damage_max);

    int get_disntance_min_attack() const;

    int use_item(Entity& target, Player& atacante, int attacker_x, int attacker_y, int target_x, int target_y, bool is_critical) override;
};

#endif
