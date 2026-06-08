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

    // Drop al morir un NPC (oro base siempre + tabla de drop extra).
    npc_drop_prob_nothing = root.at_path("npcs.drop.prob_nothing").value_or(0.80);
    npc_drop_prob_gold = root.at_path("npcs.drop.prob_gold").value_or(0.08);
    npc_drop_prob_potion = root.at_path("npcs.drop.prob_potion").value_or(0.01);
    npc_drop_prob_item = root.at_path("npcs.drop.prob_item").value_or(0.01);
    npc_gold_drop_min = root.at_path("npcs.drop.npc_gold_drop_min").value_or(0.01);
    npc_gold_drop_max = root.at_path("npcs.drop.npc_gold_drop_max").value_or(0.2);

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

    // Classes
    wizard.inteligence = root.at_path("classes.wizard.inteligence").value_or(8);
    wizard.strength = root.at_path("classes.wizard.strength").value_or(3);
    wizard.agility = root.at_path("classes.wizard.agility").value_or(8);
    wizard.endurance = root.at_path("classes.wizard.endurance").value_or(4);
    wizard.life_factor = root.at_path("classes.wizard.life_factor").value_or(2);
    wizard.mana_factor = root.at_path("classes.wizard.mana_factor").value_or(8);
    wizard.meditation_factor = static_cast<float>(root.at_path("classes.wizard.meditation_factor").value_or(0.10));

    cleric.inteligence = root.at_path("classes.cleric.inteligence").value_or(6);
    cleric.strength = root.at_path("classes.cleric.strength").value_or(8);
    cleric.agility = root.at_path("classes.cleric.agility").value_or(6);
    cleric.endurance = root.at_path("classes.cleric.endurance").value_or(8);
    cleric.life_factor = root.at_path("classes.cleric.life_factor").value_or(7);
    cleric.mana_factor = root.at_path("classes.cleric.mana_factor").value_or(6);
    cleric.meditation_factor = static_cast<float>(root.at_path("classes.cleric.meditation_factor").value_or(0.07));

    paladin.inteligence = root.at_path("classes.paladin.inteligence").value_or(5);
    paladin.strength = root.at_path("classes.paladin.strength").value_or(8);
    paladin.agility = root.at_path("classes.paladin.agility").value_or(7);
    paladin.endurance = root.at_path("classes.paladin.endurance").value_or(8);
    paladin.life_factor = root.at_path("classes.paladin.life_factor").value_or(8);
    paladin.mana_factor = root.at_path("classes.paladin.mana_factor").value_or(8);
    paladin.meditation_factor = static_cast<float>(root.at_path("classes.paladin.meditation_factor").value_or(0.04));

    warrior.inteligence = root.at_path("classes.warrior.inteligence").value_or(4);
    warrior.strength = root.at_path("classes.warrior.strength").value_or(9);
    warrior.agility = root.at_path("classes.warrior.agility").value_or(7);
    warrior.endurance = root.at_path("classes.warrior.endurance").value_or(8);
    warrior.life_factor = root.at_path("classes.warrior.life_factor").value_or(8);
    warrior.mana_factor = root.at_path("classes.warrior.mana_factor").value_or(0);
    warrior.meditation_factor = static_cast<float>(root.at_path("classes.warrior.meditation_factor").value_or(0.0));

    // Items
    espada.name = root.at_path("items.espada.name").value_or(std::string("Espada"));
    espada.price = root.at_path("items.espada.price").value_or(100);
    espada.distance = root.at_path("items.espada.distance").value_or(1);
    espada.damage_min = root.at_path("items.espada.damage_min").value_or(2);
    espada.damage_max = root.at_path("items.espada.damage_max").value_or(5);

    hacha.name = root.at_path("items.hacha.name").value_or(std::string("Hacha"));
    hacha.price = root.at_path("items.hacha.price").value_or(150);
    hacha.distance = root.at_path("items.hacha.distance").value_or(1);
    hacha.damage_min = root.at_path("items.hacha.damage_min").value_or(4);
    hacha.damage_max = root.at_path("items.hacha.damage_max").value_or(5);

    martillo.name = root.at_path("items.martillo.name").value_or(std::string("Martillo"));
    martillo.price = root.at_path("items.martillo.price").value_or(120);
    martillo.distance = root.at_path("items.martillo.distance").value_or(1);
    martillo.damage_min = root.at_path("items.martillo.damage_min").value_or(1);
    martillo.damage_max = root.at_path("items.martillo.damage_max").value_or(9);

    vara_fresno.name = root.at_path("items.vara_fresno.name").value_or(std::string("Vara de fresno"));
    vara_fresno.price = root.at_path("items.vara_fresno.price").value_or(90);
    vara_fresno.distance = root.at_path("items.vara_fresno.distance").value_or(5);
    vara_fresno.damage_min = root.at_path("items.vara_fresno.damage_min").value_or(2);
    vara_fresno.damage_max = root.at_path("items.vara_fresno.damage_max").value_or(4);
    vara_fresno.mana_cost = root.at_path("items.vara_fresno.mana_cost").value_or(5);

    flauta_elfica.name = root.at_path("items.flauta_elfica.name").value_or(std::string("Flauta elfica"));
    flauta_elfica.price = root.at_path("items.flauta_elfica.price").value_or(300);
    flauta_elfica.distance = root.at_path("items.flauta_elfica.distance").value_or(5);
    flauta_elfica.damage_min = root.at_path("items.flauta_elfica.damage_min").value_or(0);
    flauta_elfica.damage_max = root.at_path("items.flauta_elfica.damage_max").value_or(0);
    flauta_elfica.mana_cost = root.at_path("items.flauta_elfica.mana_cost").value_or(100);

    baculo_nudoso.name = root.at_path("items.baculo_nudoso.name").value_or(std::string("Baculo nudoso"));
    baculo_nudoso.price = root.at_path("items.baculo_nudoso.price").value_or(180);
    baculo_nudoso.distance = root.at_path("items.baculo_nudoso.distance").value_or(5);
    baculo_nudoso.damage_min = root.at_path("items.baculo_nudoso.damage_min").value_or(4);
    baculo_nudoso.damage_max = root.at_path("items.baculo_nudoso.damage_max").value_or(8);
    baculo_nudoso.mana_cost = root.at_path("items.baculo_nudoso.mana_cost").value_or(15);

    baculo_engarzado.name = root.at_path("items.baculo_engarzado.name").value_or(std::string("Baculo engarzado"));
    baculo_engarzado.price = root.at_path("items.baculo_engarzado.price").value_or(350);
    baculo_engarzado.distance = root.at_path("items.baculo_engarzado.distance").value_or(6);
    baculo_engarzado.damage_min = root.at_path("items.baculo_engarzado.damage_min").value_or(8);
    baculo_engarzado.damage_max = root.at_path("items.baculo_engarzado.damage_max").value_or(20);
    baculo_engarzado.mana_cost = root.at_path("items.baculo_engarzado.mana_cost").value_or(30);

    armadura_cuero.name = root.at_path("items.armadura_cuero.name").value_or(std::string("Armadura de cuero"));
    armadura_cuero.price = root.at_path("items.armadura_cuero.price").value_or(60);
    armadura_cuero.defense_min = root.at_path("items.armadura_cuero.defense_min").value_or(2);
    armadura_cuero.defense_max = root.at_path("items.armadura_cuero.defense_max").value_or(6);

    armadura_placas.name = root.at_path("items.armadura_placas.name").value_or(std::string("Armadura de placas"));
    armadura_placas.price = root.at_path("items.armadura_placas.price").value_or(500);
    armadura_placas.defense_min = root.at_path("items.armadura_placas.defense_min").value_or(15);
    armadura_placas.defense_max = root.at_path("items.armadura_placas.defense_max").value_or(30);

    tunica_azul.name = root.at_path("items.tunica_azul.name").value_or(std::string("Tunica azul"));
    tunica_azul.price = root.at_path("items.tunica_azul.price").value_or(130);
    tunica_azul.defense_min = root.at_path("items.tunica_azul.defense_min").value_or(6);
    tunica_azul.defense_max = root.at_path("items.tunica_azul.defense_max").value_or(10);

    capucha.name = root.at_path("items.capucha.name").value_or(std::string("Capucha"));
    capucha.price = root.at_path("items.capucha.price").value_or(30);
    capucha.defense_min = root.at_path("items.capucha.defense_min").value_or(1);
    capucha.defense_max = root.at_path("items.capucha.defense_max").value_or(4);

    casco_hierro.name = root.at_path("items.casco_hierro.name").value_or(std::string("Casco de hierro"));
    casco_hierro.price = root.at_path("items.casco_hierro.price").value_or(100);
    casco_hierro.defense_min = root.at_path("items.casco_hierro.defense_min").value_or(4);
    casco_hierro.defense_max = root.at_path("items.casco_hierro.defense_max").value_or(8);

    sombrero_magico.name = root.at_path("items.sombrero_magico.name").value_or(std::string("Sombrero magico"));
    sombrero_magico.price = root.at_path("items.sombrero_magico.price").value_or(220);
    sombrero_magico.defense_min = root.at_path("items.sombrero_magico.defense_min").value_or(4);
    sombrero_magico.defense_max = root.at_path("items.sombrero_magico.defense_max").value_or(12);

    escudo_tortuga.name = root.at_path("items.escudo_tortuga.name").value_or(std::string("Escudo de tortuga"));
    escudo_tortuga.price = root.at_path("items.escudo_tortuga.price").value_or(80);
    escudo_tortuga.defense_min = root.at_path("items.escudo_tortuga.defense_min").value_or(1);
    escudo_tortuga.defense_max = root.at_path("items.escudo_tortuga.defense_max").value_or(2);

    escudo_hierro.name = root.at_path("items.escudo_hierro.name").value_or(std::string("Escudo de hierro"));
    escudo_hierro.price = root.at_path("items.escudo_hierro.price").value_or(140);
    escudo_hierro.defense_min = root.at_path("items.escudo_hierro.defense_min").value_or(1);
    escudo_hierro.defense_max = root.at_path("items.escudo_hierro.defense_max").value_or(4);

    pocion_vida.name = root.at_path("items.pocion_vida.name").value_or(std::string("Pocion de vida"));
    pocion_vida.price = root.at_path("items.pocion_vida.price").value_or(50);
    pocion_vida.heal_life = root.at_path("items.pocion_vida.heal_life").value_or(100);
    pocion_vida.heal_mana = root.at_path("items.pocion_vida.heal_mana").value_or(0);

    pocion_mana.name = root.at_path("items.pocion_mana.name").value_or(std::string("Pocion de mana"));
    pocion_mana.price = root.at_path("items.pocion_mana.price").value_or(50);
    pocion_mana.heal_life = root.at_path("items.pocion_mana.heal_life").value_or(0);
    pocion_mana.heal_mana = root.at_path("items.pocion_mana.heal_mana").value_or(100);

    goblin.name           = root.at_path("npcs.goblin.name").value_or(std::string("Goblin"));
    goblin.lifepoints     = root.at_path("npcs.goblin.lifepoints").value_or(30);
    goblin.attack_dmg     = root.at_path("npcs.goblin.attack_dmg").value_or(5);
    goblin.ticks_to_spawn = root.at_path("npcs.goblin.ticks_to_spawn").value_or(100);

    spider1.name = root.at_path("npcs.spider.name").value_or(std::string("Spider1"));
    spider1.lifepoints = root.at_path("npcs.spider.lifepoints").value_or(20);
    spider1.attack_dmg = root.at_path("npcs.spider.attack_dmg").value_or(4);
    spider1.ticks_to_spawn = root.at_path("npcs.spider.ticks_to_spawn").value_or(40);

    spider2.name = root.at_path("npcs.spider.name").value_or(std::string("Spider2"));
    spider2.lifepoints = root.at_path("npcs.spider.lifepoints").value_or(30);
    spider2.attack_dmg = root.at_path("npcs.spider.attack_dmg").value_or(6);
    spider2.ticks_to_spawn = root.at_path("npcs.spider.ticks_to_spawn").value_or(40);

    spider3.name = root.at_path("npcs.spider.name").value_or(std::string("Spider3"));
    spider3.lifepoints = root.at_path("npcs.spider.lifepoints").value_or(40);
    spider3.attack_dmg = root.at_path("npcs.spider.attack_dmg").value_or(8);
    spider3.ticks_to_spawn = root.at_path("npcs.spider.ticks_to_spawn").value_or(40);

    skeleton1.name = root.at_path("npcs.skeleton.name").value_or(std::string("Skeleton1"));
    skeleton1.lifepoints = root.at_path("npcs.skeleton.lifepoints").value_or(25);
    skeleton1.attack_dmg = root.at_path("npcs.skeleton.attack_dmg").value_or(7);
    skeleton1.ticks_to_spawn = root.at_path("npcs.skeleton.ticks_to_spawn").value_or(60);

    skeleton2.name = root.at_path("npcs.skeleton.name").value_or(std::string("Skeleton2"));
    skeleton2.lifepoints = root.at_path("npcs.skeleton.lifepoints").value_or(35);
    skeleton2.attack_dmg = root.at_path("npcs.skeleton.attack_dmg").value_or(9);
    skeleton2.ticks_to_spawn = root.at_path("npcs.skeleton.ticks_to_spawn").value_or(60);

    skeleton3.name = root.at_path("npcs.skeleton.name").value_or(std::string("Skeleton3"));
    skeleton3.lifepoints = root.at_path("npcs.skeleton.lifepoints").value_or(45);
    skeleton3.attack_dmg = root.at_path("npcs.skeleton.attack_dmg").value_or(11);
    skeleton3.ticks_to_spawn = root.at_path("npcs.skeleton.ticks_to_spawn").value_or(60);

    zombie.name = root.at_path("npcs.zombie.name").value_or(std::string("Zombie"));
    zombie.lifepoints = root.at_path("npcs.zombie.lifepoints").value_or(35);
    zombie.attack_dmg = root.at_path("npcs.zombie.attack_dmg").value_or(6);
    zombie.ticks_to_spawn = root.at_path("npcs.zombie.ticks_to_spawn").value_or(80);

    orc.name = root.at_path("npcs.orc.name").value_or(std::string("Orc"));
    orc.lifepoints = root.at_path("npcs.orc.lifepoints").value_or(60);
    orc.attack_dmg = root.at_path("npcs.orc.attack_dmg").value_or(12);
    orc.ticks_to_spawn = root.at_path("npcs.orc.ticks_to_spawn").value_or(150);

    golem1.name = root.at_path("npcs.golem.name").value_or(std::string("Golem1"));
    golem1.lifepoints = root.at_path("npcs.golem.lifepoints").value_or(80);
    golem1.attack_dmg = root.at_path("npcs.golem.attack_dmg").value_or(15);
    golem1.ticks_to_spawn = root.at_path("npcs.golem.ticks_to_spawn").value_or(200);

    golem2.name = root.at_path("npcs.golem.name").value_or(std::string("Golem2"));
    golem2.lifepoints = root.at_path("npcs.golem.lifepoints").value_or(90);
    golem2.attack_dmg = root.at_path("npcs.golem.attack_dmg").value_or(25);
    golem2.ticks_to_spawn = root.at_path("npcs.golem.ticks_to_spawn").value_or(200);

    golem3.name = root.at_path("npcs.golem.name").value_or(std::string("Golem3"));
    golem3.lifepoints = root.at_path("npcs.golem.lifepoints").value_or(100);
    golem3.attack_dmg = root.at_path("npcs.golem.attack_dmg").value_or(35);
    golem3.ticks_to_spawn = root.at_path("npcs.golem.ticks_to_spawn").value_or(200);

    if (auto arr = root.at_path("zones.desert.allowed_npcs").as_array()) {
        for (auto& elem : *arr) {
            if (auto s = elem.value<std::string>()) {
                desert_npcs.push_back(*s);
            }
        }
    }
    if (auto arr = root.at_path("zones.forest.allowed_npcs").as_array()) {
        for (auto& elem : *arr) {
            if (auto s = elem.value<std::string>()) {
                forest_npcs.push_back(*s);
            }
        }
    }
    if (auto arr = root.at_path("zones.dungeon.allowed_npcs").as_array()) {
        for (auto& elem : *arr) {
            if (auto s = elem.value<std::string>()) {
                dungeon_npcs.push_back(*s);
            }
        }
    }

    clan_min_level_to_found = root.at_path("clans.min_level_to_found").value_or(6);
    clan_max_members = root.at_path("clans.max_members").value_or(16);
    clan_defense_bonus_per_member = root.at_path("clans.clan_defense_bonus_per_member").value_or(0.05);
    clan_attack_bonus_per_member = root.at_path("clans.clan_attack_bonus_per_member").value_or(0.05);

    // Gameloop
    tick_rate_ms = root.at_path("gameloop.tick_rate_ms").value_or(50);
    ticks_per_second = root.at_path("gameloop.ticks_per_second").value_or(20);    
}