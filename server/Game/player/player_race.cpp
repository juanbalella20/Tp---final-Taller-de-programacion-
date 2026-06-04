#include "player_race.h"

#include <string>

float PlayerRace::race_constitution() {
    return endurance;
}

int PlayerRace::race_inteligence() {
    return inteligence; 
}

int PlayerRace::race_agility() {
    return agility;
}

int PlayerRace::race_strength() {
    return strength; 
}

int PlayerRace::race_life_factor() {
    return life_factor;
}

int PlayerRace::race_mana_factor() {
    return mana_factor;
}

void PlayerRace::set_human() {
    inteligence = 5;
    strength = 5;
    agility = 5;
    endurance = 0.5;
    life_factor = 5;
    mana_factor = 5;
    name = "human";
}

void PlayerRace::set_elf() {
    inteligence = 7;
    strength = 5;
    agility = 7;
    endurance = 0.2;
    life_factor = 4;
    mana_factor = 6;
    name = "elf";
}

void PlayerRace::set_dwarf() {
    inteligence = 6;
    strength = 7;
    agility = 4;
    endurance = 0.7;
    life_factor = 6;
    mana_factor = 4;
    name = "dwarf";
}

void PlayerRace::set_gnome() {
    inteligence = 7;
    strength = 8;
    agility = 8;
    endurance = 0.6;
    life_factor = 2;
    mana_factor = 4;
    name = "gnome";
}

const std::string& PlayerRace::get_name() const {
    return name;
}
