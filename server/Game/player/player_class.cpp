#include "player_class.h"

#include "game_config.h"

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
    const auto& cfg = GameConfig::instance();
    inteligence = cfg.wizard.inteligence;
    strength = cfg.wizard.strength;
    agility = cfg.wizard.agility;
    endurance = cfg.wizard.endurance;
    life_factor = cfg.wizard.life_factor;
    mana_factor = cfg.wizard.mana_factor;
    meditation_factor = cfg.wizard.meditation_factor; // FClaseMeditacion, TODO balancear 
}

void PlayerClass::set_cleric() { 
    const auto& cfg = GameConfig::instance();
    inteligence = cfg.cleric.inteligence;
    strength = cfg.cleric.strength;
    agility = cfg.cleric.agility;
    endurance = cfg.cleric.endurance;
    life_factor = cfg.cleric.life_factor;
    mana_factor = cfg.cleric.mana_factor;
    meditation_factor = cfg.cleric.meditation_factor; // FClaseMeditacion, TODO balancear
}

void PlayerClass::set_paladin() {
    const auto& cfg = GameConfig::instance();
    inteligence = cfg.paladin.inteligence;
    strength = cfg.paladin.strength;
    agility = cfg.paladin.agility;
    endurance = cfg.paladin.endurance;
    life_factor = cfg.paladin.life_factor;
    mana_factor = cfg.paladin.mana_factor;
    meditation_factor = cfg.paladin.meditation_factor; // FClaseMeditacion, TODO balancear
}

void PlayerClass::set_warrior() {
    const auto& cfg = GameConfig::instance();
    inteligence = cfg.warrior.inteligence;
    strength = cfg.warrior.strength;
    agility = cfg.warrior.agility;
    endurance = cfg.warrior.endurance;
    life_factor = cfg.warrior.life_factor;
    mana_factor = cfg.warrior.mana_factor; // El guerrero no usa magia: su maná es siempre 0
    meditation_factor = cfg.warrior.meditation_factor; // El guerrero no puede meditar
}
