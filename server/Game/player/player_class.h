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
    float meditation_factor;  // FClaseMeditacion. 0 => la clase no puede meditar (guerrero).

public:
    PlayerClass() {}
    
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
