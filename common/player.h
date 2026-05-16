#ifndef PLAYER_H_
#define PLAYER_H_

#include "player_race.h"
#include "player_class.h"
#include "inventory.h"
<<<<<<< HEAD
#include "item.h"

#include <string>
=======

#include <string>
#include <memory>
>>>>>>> origin/dev

enum class PlayerStatus { ALIVE, DEAD };

class Player {

private:
    std::string name;
    int gold;
    int lives;
    int experience;
    int mana;
    int id_clan;
    int level;
    PlayerStatus status;
    std::shared_ptr<Item> equipped_item;

    PlayerRace player_race;
    PlayerClass player_class;
    Inventory player_inventory;

    int max_life();

    int max_mana();

    void level_up();

public:
    Player(const std::string name, PlayerRace player_race, PlayerClass player_class);

    void add_item(Item item);

    void drop_item(Item item);

    void equip_item(Item item);

    void unequip_item(Item item);

    void use_object(Item item);

    int damage_attack();

    void recv_attack(int damage);

    void revive();

    void heal_life(const int healthy_life);

    void heal_mana(const int healthy_mana);

    void heal(const int healthy_life, const int healthy_mana);

    void add_gold(const int extra_gold);

    bool give_gold(const int amount);

    int get_gold();

    std::string get_name();
};

#endif
