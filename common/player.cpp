#include "player.h"


Player::Player(PlayerRace player_race, PlayerClass player_class): 
    player_race(player_race), 
    player_class(player_class), 
    player_inventory() {}


void Player::take_object(Item item) {
    player_inventory.equip(item);
}

void Player::drop_object(Item item) {
    player_inventory.unequip(item);
}

void Player::use_object(Item item) {
    item.use_item();
}