#include "player.h"

#include <cmath>

Player::Player(const std::string name, PlayerRace player_race, PlayerClass player_class):
    name(name),
    status(PlayerStatus::ALIVE),
    player_race(player_race), 
    player_class(player_class), 
    player_inventory() {

    lives = calculate_max_life();
    gold = 0;
    experience = 0;
    mana = calculate_max_mana();
    level = 0;
}

int Player::calculate_max_life() {
    // VidaMax = Constitución * FClaseVida * FRazaVida * Nivel
    return player_race.life_strength() * player_class.life_strength() * level;
}

int Player::calculate_max_mana() {
    // ManaMax = Inteligencia * FClaseMana * FRazaMana * Nivel
    return player_race.mana_strength() * player_class.mana_strength() * level;
}

void Player::level_up() {
    int limit = 1000 * std::pow(level, 1.8);
    if (experience >= limit) {
        level += 1;
    }
}

void Player::add_item(Item item) {
    player_inventory.add_item(item);
}

void Player::drop_item(Item item) {
    player_inventory.drop_item(item);
}

void Player::equip_item(Item item) {
    if ( /*no tiene ningun item equipado*/ ) {
        player_inventory.unequip_item(item);
        equipped_item = item;
    } else {
        player_inventory.equip_item(equipped_item);
        player_inventory.unequip_item(item);
        equipped_item = item;
    }
}

void Player::unequip_item(Item item) {
    if ( /*no tiene ningun item equipado*/ ) {
        return;
    }

    if (equipped_item == item) {
        player_inventory.equip_item(item);
        equipped_item = /* sin item equipado */;
    }
}

void Player::use_object(Item item) {
    item.use_item();
}

void Player::revive() {
    // Vida = FRazaRecuperacion * segundos
    status = PlayerStatus::ALIVE;
    experience = 0;
}

void Player::heal() {
    // Vida = FRazaRecuperacion * segundos
    // Mana = FClaseMeditacion * Inteligencia * segundos
}

void Player::add_gold(const int extra_gold) {
    gold += extra_gold;
}

int Player::get_gold() {
    return gold;
}

std::string Player::get_name() {
    return name;
}
