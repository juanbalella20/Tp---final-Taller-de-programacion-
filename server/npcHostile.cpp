#include "npcHostile.h"
#include <iostream>
#include <random>

NPChostile::NPChostile() {
    this->state = State::ALIVE;
}

std::string NPChostile::get_name() const {
    return this->name;
}

bool NPChostile::is_dead() const {
    return this->state == State::DEAD;
}

void NPChostile::set_state(State state) {
    this->state = state;
}

void NPChostile::set_type(int type) {
    this->type = type;
}

void NPChostile::set_attack(int attack) {
    this->attack = attack;
}

void NPChostile::set_hp(int hp) {
    this->hp=hp;
    set_max_hp(hp);
}

void NPChostile::set_max_hp(int hp) {
    this->max_hp=hp;
}

int NPChostile::get_max_hp() {
    return this->max_hp;   
}

int NPChostile::get_attack() {
    return this->attack;
}

void NPChostile::set_goblin(int type) {
    set_type(type);
    switch (type) {
        case GOBLIN_LVL_1 : {
            set_hp(GO_LVL_1_hp);
            set_attack(GO_LVL_1_atk);
            set_npc_name("Goblin Level 1");
            break;
        }
        default:
            break;
    }
}

void NPChostile::set_skeleton(int type) {
    set_type(type);
    switch (type) {
        case SKE_LVL_1 : {
            set_hp(SKE_LVL_1_hp);
            set_attack(SKE_LVL_1_atk);
            set_npc_name("Skeleton Level 1");
            break;
        }
        default:
            break;
    }
}

void NPChostile::set_spider(int type) {
    set_type(type);
    switch (type) {
        case SPIDER_LVL_1 : {
            set_hp(SPIDER_LVL_1_hp);
            set_attack(SPIDER_LVL_1_atk);
            set_npc_name("Spider Level 1");
            break;
        }
        default:
            break;
    }
}

void NPChostile::set_golem(int type) {
    set_type(type);
    switch (type) {
        case GOLEM_LVL_1 : {
            set_hp(GOLEM_LVL_1_hp);
            set_attack(GOLEM_LVL_1_atk);
            set_npc_name("Golem Level 1");
            break;
        }
        default:
            break;
    }
}

void NPChostile::set_zombie() {
    set_npc_name("Zombie");
    set_attack(ZOMBIE_ATK);
    set_hp(ZOMBIE_HP);
}

void NPChostile::set_orc() {
    set_npc_name("Orc");
    set_attack(ORC_ATK);
    set_hp(ORC_HP);
}

void NPChostile::attack(Player player) {
    player.recv_attack(get_attack());
}

void NPChostile::drop() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0.0, 1.0);
    double roll = dis(gen);

    if (roll < NOTHING_CHANCE) return;
    if (roll < GOLD_CHANCE)    { 
        drop_gold();
        return; 
    }
    if (roll < POTION_CHANCE)  { 
        drop_potion(); 
        return; 
    }
    drop_item();
}



int NPChostile::drop_gold() {
    return rand(0.01, 0.2) * get_max_hp();
}

Item NPChostile::drop_item() {
    return itemdatabase.get_random_item();
}

Item NPChostile::drop_potion() {
    return itemdatabase.get_random_potion();
}

void NPChostile::death() {
    this->hp = 0;
    drop();
    set_state(State::DEAD);
}

void NPChostile::receive_damage(int dmg) {
    if (dmg >= get_hp()) {
        death();
        return;
    }
    this->hp -= dmg;
}

// es llamado por el mapa: Map.inTarget(npc, player)
// mide si el player entró en el rango de 
void NPChostile::interact(Player player, Command cmd) {
    switch (cmd.type) {
        case ATTACK :
            attack(player);
            break;
        case RECEIVE_DMG :
            recv_attack(player.damage_attack());
            break;
        default:
            break;
    }
}