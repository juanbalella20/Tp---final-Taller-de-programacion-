#ifndef PLAYER_CLASS_H_
#define PLAYER_CLASS_H_

class PlayerClass {

private:
    int inteligence;
    int agility;
    int strength;
    int endurance;
    float life_factor;
    float mana_factor;

public:
    PlayerClass() {}
    
    int class_constitution();

    int class_inteligence();

    int class_life_factor();

    int class_mana_factor();
};

#endif
