#ifndef PLAYER_RACE_H_
#define PLAYER_RACE_H_

class PlayerRace {

private:
    int inteligence;
    int agility;
    int strength;
    float endurance;
    float life_factor;
    float mana_factor;

public:
    PlayerRace() {}

    void set_human();

    void set_elf();

    void set_dwarf();

    void set_gnome();

    float race_constitution();

    int race_inteligence();

    int race_agility();

    int race_strength();

    int race_life_factor();

    int race_mana_factor();

};

#endif
