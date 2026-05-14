#ifndef PLAYER_CLASS_H_
#define PLAYER_CLASS_H_

class PlayerClass {

private:
    int inteligence;
    int agility;
    int strength;
    int endurance;

public:
    PlayerClass();
    
    int life_strength();

    int mana_strength();
};

#endif
