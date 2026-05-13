#include "player.h"


Player::Player(PlayerRace player_race, PlayerClass player_class): 
    status(PlayerStatus::ALIVE),
    player_race(player_race), 
    player_class(player_class), 
    player_inventory() {

    lives = MAX_LIVES;
    gold = 0;
    experience = 0;
    mana = MAX_MANA;
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
    lives = MAX_LIVES;
    status = PlayerStatus::ALIVE;
    experience = 0;
}

void Player::heal() {
    lives = MAX_LIVES;
    mana = MAX_MANA;
}

int Player::get_gold() {
    return gold;
}

