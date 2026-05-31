#include "player_race.h"

int PlayerRace::race_constitution() {
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
    inteligence = 10;
    strength = 10;
    agility = 10;
    endurance = 10;
    life_factor = 10;
    mana_factor = 10;
}

void PlayerRace::set_elf() {
    inteligence = 17;
    strength = 5;
    agility = 17;
    endurance = 4;
    life_factor = 4;
    mana_factor = 16;
}

void PlayerRace::set_dwarf() {
    inteligence = 6;
    strength = 17;
    agility = 4;
    endurance = 17;
    life_factor = 16;
    mana_factor = 4;
}

void PlayerRace::set_gnome() {
    inteligence = 14;
    strength = 8;
    agility = 8;
    endurance = 14;
    life_factor = 12;
    mana_factor = 14;
}
