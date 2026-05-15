#include "player_class.h"

int PlayerClass::class_constitution() {
    return strength + endurance + agility;
}

int PlayerClass::class_inteligence() {
    return inteligence; 
}

int PlayerClass::class_life_factor() {
    return life_factor;
}

int PlayerClass::class_mana_factor() {
    return mana_factor;
}
