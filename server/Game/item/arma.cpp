#include "arma.h"
#include "../entity.h"
#include <cmath>

Arma::Arma(const std::string& id, const std::string& name, int price, int disntance_min_attack, int damage):
        Item(id, name, price), disntance_min_attack(disntance_min_attack), damage(damage) {}

int Arma::get_disntance_min_attack() const {
    return disntance_min_attack;
}

void Arma::use_item(Entity& target, int attacker_x, int attacker_y, int target_x, int target_y) {
    int dx = std::abs(attacker_x - target_x);
    int dy = std::abs(attacker_y - target_y);
    if (dx + dy > disntance_min_attack) return;
    target.receive_damage(damage);
}
