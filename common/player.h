#ifndef PLAYER_H_
#define PLAYER_H_

#include "player_race.h"
#include "player_class.h"
#include "inventory.h"

class Player {

private:
    int gold;
    int lives;
    int experience;
    int mana;
    int id_clan;

    PlayerRace player_race;
    PlayerClass player_class;
    Inventory player_inventory;

public:
    Player(PlayerRace player_race, PlayerClass player_class);

    void take_object(Item item);

    void drop_object(Item item);

    void use_object(Item item);
};

#endif
