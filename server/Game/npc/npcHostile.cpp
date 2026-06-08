#include "npcHostile.h"
#include "../player/player.h"
#include "../zoneWorld.h"
#include <cstdlib>
#include <vector>
#include <iostream>

NPChostile::NPChostile(const std::string& type_id, const std::string& name,
                       int lifepoints, int attack_dmg, int ticks_to_spawn)
    : type_id(type_id),
      lifepoints(lifepoints), max_lifepoints(lifepoints), attack_dmg(attack_dmg),
      state(State::ALIVE), coord_x(0), coord_y(0),
      remaining_ticks_to_spawn(0), ticks_to_spawn(ticks_to_spawn),
      attack_speed_ticks(20),
      current_attack_cooldown(0) {
    this->name = name;
}

std::string NPChostile::get_name() const { return this->name; }
const std::string& NPChostile::get_type_id() const { return type_id; }

void NPChostile::set_position(int x, int y) { coord_x = x; coord_y = y; }
int NPChostile::get_coord_x() const { return coord_x; }
int NPChostile::get_coord_y() const { return coord_y; }

bool NPChostile::is_dead() const { return state == State::DEAD; }

bool NPChostile::can_spawn() const {
    return state == State::DEAD && remaining_ticks_to_spawn == 0;
}

void NPChostile::set_state(State s) { this->state = s; }

int NPChostile::drop() {
    // Oro = rand(0, 0.2) * VidaMaxNPC
    double factor = (std::rand() / static_cast<double>(RAND_MAX)) * 0.2;
    return static_cast<int>(factor * max_lifepoints);
}

void NPChostile::death() {
    lifepoints = 0;
    set_state(State::DEAD);
    remaining_ticks_to_spawn = ticks_to_spawn;
}

void NPChostile::reduce_ticks_to_spawn() {
    if (remaining_ticks_to_spawn > 0) {
        remaining_ticks_to_spawn--;
    }
}

void NPChostile::revive(int x, int y) {
    lifepoints = max_lifepoints;
    set_position(x, y);
    set_state(State::ALIVE);
}

DamageOutcome NPChostile::receive_damage(int dmg, Player& atacante, bool is_critical) {
    (void)is_critical;  // los NPC no esquivan ni tienen defensa modelada aún
    lifepoints -= dmg;
    bool murio = false;
    int gold_drop = 0;
    if (lifepoints <= 0) {
        gold_drop = drop();  // oro a tirar; se retorna recién al final
        death();
        murio = true;
    }
    const int nivel_npc = 1;
    // La XP se otorga SIEMPRE, haya muerto o no, antes de retornar el oro.
    bool level_up =atacante.ganar_xp(dmg, nivel_npc, murio, max_lifepoints);
    return {dmg, gold_drop, false, level_up};
}

void NPChostile::move_towards(int target_x, int target_y, ZoneWorld& world, const std::vector<Player*>& players) {
    int dx = target_x - coord_x;
    int dy = target_y - coord_y;

    int next_x = coord_x;
    int next_y = coord_y;

    if (std::abs(dx) > std::abs(dy)) {
        next_x += (dx > 0) ? 1 : -1;
    } else if (std::abs(dy) > 0) {
        next_y += (dy > 0) ? 1 : -1;
    }

    if (!world.is_blocked_terrain(next_x, next_y) && 
        !world.has_actor_at(next_x, next_y, players)) {
        
        coord_x = next_x;
        coord_y = next_y;
    
        std::cout << "[DEBUG] NPC " << get_name() << " stepped to (" 
                  << coord_x << "," << coord_y << ")" << std::endl;
    }
}

bool NPChostile::can_attack() const {
    // Puede atacar si su tiempo de espera llegó a cero
    return current_attack_cooldown <= 0;
}

void NPChostile::reset_attack_cooldown() {
    // Reinicia el contador a su velocidad base
    current_attack_cooldown = attack_speed_ticks;
}

void NPChostile::tick_cooldowns() {
    // Si el contador es mayor a cero, lo reducimos
    if (current_attack_cooldown > 0) {
        current_attack_cooldown--;
    }
}

int NPChostile::get_damage() const {
    return attack_dmg;
}

/*

void NPChostile::set_type(int t) { this->type = t; }
void NPChostile::set_attack(int atk) { this->attack_dmg = atk; }
int NPChostile::get_attack() { return attack_dmg; }

void NPChostile::set_goblin(int type) {
    set_type(type);
    lifepoints = 30;
    set_attack(5);
    this->name = "Goblin";
}

void NPChostile::set_skeleton(int type) {
    set_type(type);
    lifepoints = 25;
    set_attack(7);
    this->name = "Skeleton";
}

void NPChostile::set_spider(int type) {
    set_type(type);
    lifepoints = 20;
    set_attack(4);
    this->name = "Spider";
}

void NPChostile::set_golem(int type) {
    set_type(type);
    lifepoints = 80;
    set_attack(15);
    this->name = "Golem";
}

void NPChostile::set_zombie() {
    lifepoints = 40;
    set_attack(8);
    this->name = "Zombie";
}

void NPChostile::set_orc() {
    lifepoints = 60;
    set_attack(12);
    this->name = "Orc";
}




*/