#ifndef PLAYER_RACE_H_
#define PLAYER_RACE_H_

#include <string>

#include "../../../common/constants/game_constants.h"

class PlayerRace {

private:
    int inteligence;
    int agility;
    int strength;
    float endurance;
    float life_factor;
    float mana_factor;
    std::string name;
    RaceType race_id;  // identidad de la raza (dominio), para persistir

public:
    PlayerRace() {}

    // Construye una raza ya configurada a partir de su nombre ("human", "elf",
    // "dwarf", "gnome"). Nombre desconocido => human (fallback seguro). Unico
    // lugar que mapea string de raza -> set_xxx().
    static PlayerRace from_name(const std::string& name);

    // Identidad de la raza como enum del dominio (para persistir).
    RaceType get_race_id() const;

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

    const std::string& get_name() const;

};

#endif
