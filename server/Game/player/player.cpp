#include "player.h"
#include "../../game_exceptions.h"
#include "alive_state.h"
#include "ghost_state.h"

#include <cmath>

Player::Player(const std::string name, PlayerRace& player_race, PlayerClass& player_class):
    name(name),
    state(std::make_unique<AliveState>()),
    equipped_item(nullptr),
    player_race(player_race),
    player_class(player_class),
    player_inventory(),
    level(1) {

    lives = max_life();
    gold = 10000;
    experience = 0;
    mana = max_mana();
}

void Player::to_ghost() {
    state = std::make_unique<GhostState>();
}

void Player::to_alive() {
    state = std::make_unique<AliveState>();
}

const Inventory& Player::get_inventory() const {
    return player_inventory;
}

uint32_t Player::max_life() {
    // VidaMax = Constitución * FClaseVida * FRazaVida * Nivel
    return player_race.race_constitution() * player_class.class_life_factor() * player_race.race_life_factor() * level.get_number();
}

uint32_t Player::max_mana() {
    // ManaMax = Inteligencia * FClaseMana * FRazaMana * Nivel
    return (player_race.race_inteligence() + player_class.class_inteligence()) * player_class.class_mana_factor() * player_race.race_mana_factor() * level.get_number();
}


void Player::add_item(std::unique_ptr<Item> item) {
    player_inventory.add_item(std::move(item));
}

/* TODO: implement ITEM */


void Player::drop_item(Item* item) {
    player_inventory.drop_item(item);
}

std::vector<std::unique_ptr<Item>> Player::drop_inventory() {
    return player_inventory.drop_all();
}

/*

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



void Player::revive() {
    state->revive(*this);
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

uint32_t Player::get_gold() const {
    return gold;
}

std::string Player::get_name() const {
    return name;
}

int Player::get_coord_x() const {
    return coord_x;
}

int Player::get_coord_y() const {
    return coord_y;
}

void Player::update_position(const int x, const int y) {
    coord_x = x;
    coord_y = y;
}

bool Player::is_ghost() const {
    return state->is_ghost();
}

bool Player::is_dead() const {
    return state->is_ghost();
}

bool Player::can_interact() const {
    return state->can_interact();
}

void Player::set_ghost() {
    to_ghost();
}

bool Player::is_meditating() const {
    return meditating;
}
 
void Player::change_meditation() {
    meditating = !meditating;
}
 
void Player::stop_meditation() {
    meditating = false;
}
/* 
bool Player::can_meditate() const {
    return player_class.class_can_meditate(); //Tengoq ue actualizar player_class, solo el guerrero no puede meditar
}
*/

uint32_t Player::get_lives() const {
    return lives;
}

uint32_t Player::get_xp() const {
    return experience;
}

uint32_t Player::get_mana() const {
    return mana;
}
 
int Player::get_level() const {
    return level.get_number();
}

bool Player::is_newbie() const {
    return level.is_newbie();
}

bool Player::can_attack_level(int other_level) const {
    return level.can_attack_level(other_level);
}
 
int Player::get_clan_id() const {
    return id_clan;
}

int Player::damage_attack() {
    // Daño = Fuerza * rand(DañoArmaMin, DañoArmaMax)
    // TODO: cuando Item este definido, usar el arma equipada
    return player_race.race_strength() + player_class.class_strength();
}
 
int Player::receive_damage(int damage, Player& atacante) {
    return state->receive_damage(*this, damage, atacante);
}

int Player::attack(Entity& target, int target_x, int target_y) {
    return state->attack(*this, target, target_x, target_y);
}

void Player::ganar_xp(int dano, int nivel_target, bool murio, int vida_max_target) {
    experience += level.xp_per_attack(dano, nivel_target);
    if (murio)
        experience += level.xp_per_kill(vida_max_target, nivel_target);
    check_level_up();
}
 
void Player::add_experience(int exp) {
    experience += exp;
}

void Player::equip_item(std::string item_id) {
    player_inventory.equip_item(item_id);
}
 
void Player::check_level_up() {
    if (level.try_level_up(experience)) {
        lives = max_life();
        mana  = max_mana();
    }
}


