#ifndef GAME_CONFIG_H_
#define GAME_CONFIG_H_

#include <string>

struct RaceConfig {
    int inteligence;
    int strength;
    int agility;
    float endurance;
    int life_factor;
    int mana_factor;
};

struct ClassConfig {
    int inteligence;
    int strength;
    int agility;
    int endurance;
    int life_factor;
    int mana_factor;
    float meditation_factor;
};

class GameConfig {
public:
    // Level
    double xp_base;
    double xp_exponent;
    int    xp_level_offset;
    double xp_kill_factor;
    double gold_base;
    double gold_exponent;
    double gold_excess_factor;
    int    newbie_max_level;
    int    level_diff_max;

    // Combat
    double crit_chance;
    double dodge_threshold;
    double npc_gold_drop_max;

    // Races
    RaceConfig human;
    RaceConfig elf;
    RaceConfig dwarf;
    RaceConfig gnome;

    // Classes
    ClassConfig wizard;
    ClassConfig cleric;
    ClassConfig paladin;
    ClassConfig warrior;

    static GameConfig& instance();
    void load(const std::string& toml_path);

private:
    GameConfig() = default;
    GameConfig(const GameConfig&) = delete;
    GameConfig& operator=(const GameConfig&) = delete;
};

#endif