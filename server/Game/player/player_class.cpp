#include "player_class.h"

int PlayerClass::class_constitution() {
    return endurance;
}

int PlayerClass::class_inteligence() {
    return inteligence; 
}

int PlayerClass::class_agility() {
    return agility; 
}

int PlayerClass::class_strength() {
    return strength; 
}

int PlayerClass::class_life_factor() {
    return life_factor;
}

int PlayerClass::class_mana_factor() {
    return mana_factor;
}

void PlayerClass::set_wizard() {
    inteligence = 18;
    strength = 3;
    agility = 8;
    endurance = 4;
    life_factor = 2;
    mana_factor = 18;
}

void PlayerClass::set_cleric() {
    inteligence = 14;
    strength = 10;
    agility = 10;
    endurance = 12;
    life_factor = 8;
    mana_factor = 12;
}

void PlayerClass::set_paladin() {
    inteligence = 10;
    strength = 15;
    agility = 10;
    endurance = 15;
    life_factor = 14;
    mana_factor = 8;
}

void PlayerClass::set_warrior() {
    inteligence = 3;
    strength = 18;
    agility = 12;
    endurance = 18;
    life_factor = 18;
    mana_factor = 1;
}
