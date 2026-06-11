#include "potion.h"

#include "../player/player.h"

Potion::Potion(const std::string& id, const std::string& name, int price,
               int heal_life, int heal_mana)
    : Item(id, name, price), heal_life(heal_life), heal_mana(heal_mana) {}

int Potion::get_heal_life() const { return heal_life; }
int Potion::get_heal_mana() const { return heal_mana; }

DamageOutcome Potion::use_item(Entity& target, Player& atacante,
                               int attacker_x, int attacker_y,
                               int target_x, int target_y, bool is_critical) {
    (void)target;
    (void)attacker_x;
    (void)attacker_y;
    (void)target_x;
    (void)target_y;
    (void)is_critical;
    atacante.heal(heal_life, heal_mana);
    return {};
}
