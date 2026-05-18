#include "player.h"

#include <cmath>

Player::Player(const std::string name, PlayerRace player_race, PlayerClass player_class):
    name(name),
    status(PlayerStatus::ALIVE),
    equipped_item(nullptr),
    player_race(player_race), 
    player_class(player_class), 
    player_inventory() {

    lives = max_life();
    gold = 0;
    experience = 0;
    mana = max_mana();
    level = 0;
}

int Player::max_life() {
    // VidaMax = Constitución * FClaseVida * FRazaVida * Nivel
    return player_race.race_constitution() * player_class.class_constitution() * player_class.class_life_factor() * player_race.race_life_factor() * level;
}

int Player::max_mana() {
    // ManaMax = Inteligencia * FClaseMana * FRazaMana * Nivel
    return player_race.race_inteligence() * player_class.class_inteligence() * player_class.class_mana_factor() * player_race.race_mana_factor() * level;
}

void Player::level_up() {
    int limit = 1000 * std::pow(level, 1.8);
    if (experience >= limit) {
        level += 1;
    }
}

/* TODO: implement ITEM */

/*
void Player::add_item(Item item) {
    player_inventory.add_item(item);
}

void Player::drop_item(Item item) {
    player_inventory.drop_item(item);
}

void Player::equip_item(Item item) {
    if (equipped_item == nullptr) {
        player_inventory.unequip_item(item);
        equipped_item = item;
    } else {
        player_inventory.equip_item(equipped_item);
        player_inventory.unequip_item(item);
        equipped_item = item;
    }
}

void Player::unequip_item(Item item) {
    if (equipped_item == nullptr) {
        return;
    }

    if (equipped_item == item) {
        player_inventory.equip_item(item);
        equipped_item = nullptr;
    }
}

void Player::use_object(Item item) {
    item.use_item();
}
*/

int Player::damage_attack() {
    // Daño = Fuerza * rand(DañoArmaMin, DañoArmaMax)
    return 0;
}

void Player::recv_attack(int damage) {
    // Esquivar si rand(0, 1) ^ Agilidad < 0.001

    // Defensa = rand(ArmaduraMin, ArmaduraMax) + rand(EscudoMin, EscudoMax) + rand(CascoMin, CascoMax)

    // lives -= (damage - defense)
}

void Player::revive() {
    lives = max_life();
    mana = max_mana();
    status = PlayerStatus::ALIVE;
    experience = 0;
}

void Player::heal_life(const int healthy_life) {
    if (lives + healthy_life < max_life()) {
        lives += healthy_life;
    } else {
        lives = max_life();
    }
}

void Player::heal_mana(const int healthy_mana) {
    if (mana + healthy_mana < max_mana()) {
        mana += healthy_mana;
    } else {
        mana = max_mana();
    }
}

void Player::heal(const int healthy_life, const int healthy_mana) {
    heal_life(healthy_life);
    heal_mana(healthy_mana);
}

void Player::add_gold(const int extra_gold) {
    gold += extra_gold;
}

bool Player::give_gold(const int amount) {
    if (gold >= amount) {
        gold -= amount;
        return true;
    }

    return false;
}

int Player::get_gold() {
    return gold;
}

std::string Player::get_name() const {
    return name;
}

int Player::get_coord_x() {
    return coord_x;
}

int Player::get_coord_y() {
    return coord_y;
}

void Player::update_position(const int x, const int y) {
    coord_x = x;
    coord_y = y;
}
