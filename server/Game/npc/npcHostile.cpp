#include "npcHostile.h"
#include "../player/player.h"

NPChostile::NPChostile(const std::string& type_id, const std::string& name,
                       int lifepoints, int attack_dmg, int ticks_to_spawn)
    : type_id(type_id),
      lifepoints(lifepoints), max_lifepoints(lifepoints), attack_dmg(attack_dmg),
      state(State::ALIVE), coord_x(0), coord_y(0),
      remaining_ticks_to_spawn(0), ticks_to_spawn(ticks_to_spawn) {
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

void NPChostile::drop() {}

void NPChostile::death() {
    lifepoints = 0;
    drop();
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

int NPChostile::receive_damage(int dmg, Player& atacante, bool is_critical) {
    (void)is_critical;  // los NPC no esquivan ni tienen defensa modelada aún
    lifepoints -= dmg;
    bool murio = false;
    if (lifepoints <= 0) {
        death();
        murio = true;
    }
    const int nivel_npc = 1;
    atacante.ganar_xp(dmg, nivel_npc, murio, max_lifepoints);
    return 0;
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