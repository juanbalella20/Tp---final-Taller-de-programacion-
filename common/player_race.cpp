#include "player_race.h"

int PlayerRace::race_constitution() {
    return strength + endurance + agility;
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
