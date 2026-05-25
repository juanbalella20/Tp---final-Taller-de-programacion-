#include "npcHostile.h"

NPChostile::NPChostile() : lifepoints(0), attack_dmg(0), type(0), state(State::ALIVE), coord_x(0), coord_y(0) {}

std::string NPChostile::get_name() const { return this->name; }

void NPChostile::set_position(int x, int y) { coord_x = x; coord_y = y; }
int NPChostile::get_coord_x() const { return coord_x; }
int NPChostile::get_coord_y() const { return coord_y; }

bool NPChostile::is_dead() const { return state == State::DEAD; }

void NPChostile::set_state(State s) { this->state = s; }
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

void NPChostile::drop() {}

void NPChostile::death() {
    lifepoints = 0;
    drop();
    set_state(State::DEAD);
}

void NPChostile::receive_damage(int dmg) {
    lifepoints -= dmg;
    if (lifepoints <= 0) {
        death();
    }
}
