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
}