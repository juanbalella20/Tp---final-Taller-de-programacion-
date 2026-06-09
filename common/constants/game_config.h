#ifndef GAME_CONFIG_H_
#define GAME_CONFIG_H_

#include <map>
#include <string>
#include <vector>

#include "protocol_constants.h"  // enum Zone

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
    int level;
};

class GameConfig {
public:
    // Level
    double xp_base;
    double xp_exponent;
    int xp_level_offset;
    double xp_kill_factor;
    double gold_base;
    double gold_exponent;
    double gold_excess_factor;
    int newbie_max_level;
    int level_diff_max;

    // Combat
    double crit_chance;
    double dodge_threshold;

    // Drop al morir un NPC (ver [npcs.drop] en config.toml).
    // Probabilidades de la tabla de drop EXTRA (sobre el oro base que cae siempre).
    double npc_drop_prob_nothing;
    double npc_drop_prob_gold;
    double npc_drop_prob_potion;
    double npc_drop_prob_item;
    // Factor de oro: rand(min, max) * VidaMaxNPC.
    double npc_gold_drop_min;
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

    std::vector<std::string> desert_npcs;
    std::vector<std::string> forest_npcs;
    std::vector<std::string> dungeon_npcs;

    // --- Receta de poblado por zona (bloque [zones.<nombre>] de config.toml) ---
    // Ruta al .bin de cada zona (zones.<nombre>.map).
    std::map<Zone, std::string> zone_map_paths;
    // Tipos de NPC hostil permitidos por zona (zones.<nombre>.allowed_npcs).
    std::map<Zone, std::vector<std::string>> zone_allowed_npcs;
    // Cantidades de spawn inicial.
    std::map<Zone, int> zone_num_npc;
    std::map<Zone, int> zone_num_items;
    // Cantidad de NPCs amigos adicionales en celdas random.
    std::map<Zone, int> zone_num_priests;
    std::map<Zone, int> zone_num_sellers;
    std::map<Zone, int> zone_num_bankers;

    // Clan
    int clan_min_level_to_found;
    int clan_max_members;
    double clan_defense_bonus_per_member;
    double clan_attack_bonus_per_member;

    //Gameloop
    int tick_rate_ms;
    int ticks_per_second;

    static GameConfig& instance();
    void load(const std::string& toml_path);

    // Ruta al .bin de una zona, o "" si no esta configurada.
    const std::string& zone_map_path(Zone zone) const;

private:
    GameConfig() = default;
    GameConfig(const GameConfig&) = delete;
    GameConfig& operator=(const GameConfig&) = delete;
};

#endif