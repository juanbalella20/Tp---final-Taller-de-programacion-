#ifndef PLAYER_RACE_H_
#define PLAYER_RACE_H_

class PlayerRace {

private:
    int inteligence;
    int agility;
    int strength;
    int endurance;

public:
    PlayerRace();

    int life_strength();

    int mana_strength();

};

#endif
