#include "player_race.h"

#include <string>

#include "game_config.h"

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
    const auto& cfg = GameConfig::instance();
    inteligence = cfg.human.inteligence;
    strength = cfg.human.strength;
    agility = cfg.human.agility;
    endurance = cfg.human.endurance;
    life_factor = cfg.human.life_factor;
    mana_factor = cfg.human.mana_factor;
    name = "human";
    race_id = RaceType::HUMAN;
}

void PlayerRace::set_elf() {
    const auto& cfg = GameConfig::instance();
    inteligence = cfg.elf.inteligence;
    strength = cfg.elf.strength;
    agility = cfg.elf.agility;
    endurance = cfg.elf.endurance;
    life_factor = cfg.elf.life_factor;
    mana_factor = cfg.elf.mana_factor;
    name = "elf";
    race_id = RaceType::ELF;
}

void PlayerRace::set_dwarf() {
    const auto& cfg = GameConfig::instance();
    inteligence = cfg.dwarf.inteligence;
    strength = cfg.dwarf.strength;
    agility = cfg.dwarf.agility;
    endurance = cfg.dwarf.endurance;
    life_factor = cfg.dwarf.life_factor;
    mana_factor = cfg.dwarf.mana_factor;
    name = "dwarf";
    race_id = RaceType::DWARF;
}

void PlayerRace::set_gnome() {
    const auto& cfg = GameConfig::instance();
    inteligence = cfg.gnome.inteligence;
    strength = cfg.gnome.strength;
    agility = cfg.gnome.agility;
    endurance = cfg.gnome.endurance;
    life_factor = cfg.gnome.life_factor;
    mana_factor = cfg.gnome.mana_factor;
    name = "gnome";
    race_id = RaceType::GNOME;
}

PlayerRace PlayerRace::from_name(const std::string& name) {
    PlayerRace race;
    if (name == "elf") {
        race.set_elf();
    } else if (name == "dwarf") {
        race.set_dwarf();
    } else if (name == "gnome") {
        race.set_gnome();
    } else {
        race.set_human();  // fallback: human
    }
    return race;
}

const std::string& PlayerRace::get_name() const {
    return name;
}

RaceType PlayerRace::get_race_id() const {
    return race_id;
}
