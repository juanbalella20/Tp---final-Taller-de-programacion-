#include "player_class.h"

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

float PlayerClass::class_meditation_factor() const {
    return meditation_factor;
}

bool PlayerClass::class_can_meditate() const {
    return meditation_factor > 0.0f;
}

void PlayerClass::set_wizard() {
    inteligence = 8;
    strength = 3;
    agility = 8;
    endurance = 4;
    life_factor = 2;
    mana_factor = 8;
    meditation_factor = 0.10f;  // FClaseMeditacion, TODO balancear
}

void PlayerClass::set_cleric() {
    inteligence = 6;
    strength = 8;
    agility = 6;
    endurance = 8;
    life_factor = 7;
    mana_factor = 6;
    meditation_factor = 0.07f;  // FClaseMeditacion, TODO balancear
}

void PlayerClass::set_paladin() {
    inteligence = 5;
    strength = 8;
    agility = 7;
    endurance = 8;
    life_factor = 8;
    mana_factor = 8;
    meditation_factor = 0.04f;  // FClaseMeditacion, TODO balancear
}

void PlayerClass::set_warrior() {
    inteligence = 4;
    strength = 9;
    agility = 7;
    endurance = 8;
    life_factor = 8;
    mana_factor = 0;  // El guerrero no usa magia: su maná es siempre 0
    meditation_factor = 0.0f;  // El guerrero no puede meditar
}
