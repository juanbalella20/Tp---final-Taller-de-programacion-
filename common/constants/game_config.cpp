#define TOML_IMPLEMENTATION
#include "toml.hpp"
#include "game_config.h"
#include <stdexcept>

GameConfig& GameConfig::instance() {
    static GameConfig cfg;
    return cfg;
}

void GameConfig::load(const std::string& toml_path) {
    toml::table root;
    try {
        root = toml::parse_file(toml_path);
    } catch (const toml::parse_error& e) {
        throw std::runtime_error(
            std::string("Error al parsear config.toml: ") + e.what());
    }

    // Level
    xp_base = root.at_path("level.xp_base").value_or(1000.0);
    xp_exponent = root.at_path("level.xp_exponent").value_or(1.8);
    xp_level_offset = root.at_path("level.xp_level_offset").value_or(10);
    xp_kill_factor = root.at_path("level.xp_kill_factor").value_or(0.1);
    gold_base = root.at_path("level.gold_base").value_or(100.0);
    gold_exponent = root.at_path("level.gold_exponent").value_or(1.1);
    gold_excess_factor = root.at_path("level.gold_excess_factor").value_or(1.5);
    newbie_max_level = root.at_path("level.newbie_max_level").value_or(12);
    level_diff_max = root.at_path("level.level_diff_max").value_or(10);

    // Combat
    crit_chance = root.at_path("combat.crit_chance").value_or(0.1);
    dodge_threshold = root.at_path("combat.dodge_threshold").value_or(0.001);
    npc_gold_drop_max = root.at_path("combat.npc_gold_drop_max").value_or(0.2);

    // Races
    human.inteligence = root.at_path("races.human.inteligence").value_or(5);
    human.strength = root.at_path("races.human.strength").value_or(5);
    human.agility = root.at_path("races.human.agility").value_or(5);
    human.endurance = static_cast<float>(root.at_path("races.human.endurance").value_or(0.5));
    human.life_factor = root.at_path("races.human.life_factor").value_or(5);
    human.mana_factor  = root.at_path("races.human.mana_factor").value_or(5);

    elf.inteligence = root.at_path("races.elf.inteligence").value_or(7);
    elf.strength = root.at_path("races.elf.strength").value_or(5);
    elf.agility = root.at_path("races.elf.agility").value_or(7);
    elf.endurance = static_cast<float>(root.at_path("races.elf.endurance").value_or(0.2));
    elf.life_factor = root.at_path("races.elf.life_factor").value_or(4);
    elf.mana_factor = root.at_path("races.elf.mana_factor").value_or(6);

    dwarf.inteligence = root.at_path("races.dwarf.inteligence").value_or(6);
    dwarf.strength = root.at_path("races.dwarf.strength").value_or(7);
    dwarf.agility = root.at_path("races.dwarf.agility").value_or(4);
    dwarf.endurance = static_cast<float>(root.at_path("races.dwarf.endurance").value_or(0.7));
    dwarf.life_factor = root.at_path("races.dwarf.life_factor").value_or(6);
    dwarf.mana_factor = root.at_path("races.dwarf.mana_factor").value_or(4);

    gnome.inteligence = root.at_path("races.gnome.inteligence").value_or(7);
    gnome.strength = root.at_path("races.gnome.strength").value_or(8);
    gnome.agility = root.at_path("races.gnome.agility").value_or(8);
    gnome.endurance = static_cast<float>(root.at_path("races.gnome.endurance").value_or(0.6));
    gnome.life_factor = root.at_path("races.gnome.life_factor").value_or(2);
    gnome.mana_factor = root.at_path("races.gnome.mana_factor").value_or(4);
}