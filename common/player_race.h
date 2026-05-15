#ifndef PLAYER_RACE_H_
#define PLAYER_RACE_H_

class PlayerRace {

private:
    int inteligence;
    int agility;
    int strength;
    int endurance;
    float life_factor;
    float mana_factor;

public:
    PlayerRace() {};

    int race_constitution();

    int race_inteligence();

    int race_life_factor();

    int race_mana_factor();

};

#endif
