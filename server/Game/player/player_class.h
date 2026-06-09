#ifndef PLAYER_CLASS_H_
#define PLAYER_CLASS_H_

#include <string>

#include "../../../common/constants/game_constants.h"

class PlayerClass {

private:
    int inteligence;
    int agility;
    int strength;
    int endurance;
    float life_factor;
    float mana_factor;
    float meditation_factor;  // FClaseMeditacion. 0 => la clase no puede meditar (guerrero).
    ClassType class_id;       // identidad de la clase (dominio), para persistir

public:
    PlayerClass() {}

    // Construye una clase ya configurada a partir de su nombre ("wizard",
    // "cleric", "paladin", "warrior"). Nombre desconocido => warrior (fallback
    // seguro). Unico lugar que mapea string de clase -> set_xxx().
    static PlayerClass from_name(const std::string& name);

    // Identidad de la clase como enum del dominio (para persistir).
    ClassType get_class_id() const;

    void set_wizard();

    void set_cleric();

    void set_paladin();

    void set_warrior();

    int class_constitution();

    int class_inteligence();

    int class_agility();

    int class_strength();

    int class_life_factor();

    int class_mana_factor();

    // FClaseMeditacion: maná recuperado por segundo de meditación, por punto de
    // inteligencia. Devuelve float (no int) porque es un valor fraccionario.
    float class_meditation_factor() const;

    // ¿La clase puede meditar? Solo el guerrero no puede (factor 0).
    bool class_can_meditate() const;
};

#endif
