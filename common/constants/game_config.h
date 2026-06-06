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

struct WeaponConfig {
    std::string name;
    int price;
    int distance;
    int damage_min;
    int damage_max;
};

struct MagicWeaponConfig {
    std::string name;
    int price;
    int distance;
    int damage_min;
    int damage_max;
    int mana_cost;
};

struct DefenseItemConfig {
    std::string name;
    int price;
    int defense_min;
    int defense_max;
};

struct PotionConfig {
    std::string name;
    int price;
    int heal_life;
    int heal_mana;
};

struct NpcHostileConfig {
    std::string name;
    int lifepoints;
    int attack_dmg;
    int ticks_to_spawn;
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

    // Items
    // Weapons
    WeaponConfig espada;
    WeaponConfig hacha;
    WeaponConfig martillo;
    // Magic Weapons
    MagicWeaponConfig vara_fresno;
    MagicWeaponConfig flauta_elfica;
    MagicWeaponConfig baculo_nudoso;
    MagicWeaponConfig baculo_engarzado;
    // Armours
    DefenseItemConfig armadura_cuero;
    DefenseItemConfig armadura_placas;
    DefenseItemConfig tunica_azul;
    // Helmets
    DefenseItemConfig capucha;
    DefenseItemConfig casco_hierro;
    DefenseItemConfig sombrero_magico;
    // Shields
    DefenseItemConfig escudo_tortuga;
    DefenseItemConfig escudo_hierro;
    // Potions
    PotionConfig pocion_vida;
    PotionConfig pocion_mana;

    // Npc hostile
    NpcHostileConfig goblin;
    NpcHostileConfig spider;
    NpcHostileConfig skeleton;
    NpcHostileConfig zombie;
    NpcHostileConfig orc;
    NpcHostileConfig golem;

    static GameConfig& instance();
    void load(const std::string& toml_path);

private:
    GameConfig() = default;
    GameConfig(const GameConfig&) = delete;
    GameConfig& operator=(const GameConfig&) = delete;
};

#endif