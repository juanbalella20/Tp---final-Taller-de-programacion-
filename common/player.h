#ifndef PLAYER_H_
#define PLAYER_H_

#include "player_race.h"
#include "player_class.h"
#include "inventory.h"

#include <string>

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
    Item equipped_item;

    PlayerRace player_race;
    PlayerClass player_class;
    Inventory player_inventory;

    int calculate_max_life();

    int calculate_max_mana();

    void level_up();

public:
    Player(const std::string name, PlayerRace player_race, PlayerClass player_class);

    void add_item(Item item);

    void drop_item(Item item);

    void equip_item(Item item);

    void unequip_item(Item item);

    void use_object(Item item);

    void revive();

    void heal();

    void restore_lives(); // to-do

    void restore_mana(); // to-do

    void add_gold(const int extra_gold);

    int get_gold();

    std::string get_name();
};

#endif
