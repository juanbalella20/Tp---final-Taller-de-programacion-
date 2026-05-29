#ifndef ARMA_H
#define ARMA_H

#include <string>

#include "item.h"
#include "../entity.h"

class Arma: public Item {
private:
    int disntance_min_attack;
    int damage;

public:
    Arma(const std::string& id, const std::string& name, int price, int disntance_min_attack, int damage);

    int get_disntance_min_attack() const;

    void use_item(Entity& target, int attacker_x, int attacker_y, int target_x, int target_y) override;
};

#endif
