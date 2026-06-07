#ifndef ENTITY_H_
#define ENTITY_H_

#include <string>

class Player;

// Resultado de un golpe sobre un target. Desambigua los tres casos que antes
// se mezclaban en un solo int de retorno:
//   - damage: daño efectivo aplicado (post-defensa). 0 si esquivó o no aplicó.
//   - gold_drop: oro que el target droppea al suelo al morir (0 si no murió).
//   - dodged: el target esquivó el ataque (solo Player no-crítico).
struct DamageOutcome {
    int  damage = 0;
    int  gold_drop = 0;
    bool dodged = false;
    bool level_up = false;
};

class Entity {
public:
    virtual ~Entity() = default;

    virtual std::string get_name() const = 0;

    // El target recibe daño y, con sus propios atributos (nivel, vida máxima),
    // recompensa al atacante con la XP correspondiente. Devuelve el daño hecho,
    // el oro dropeado y si esquivó (ver DamageOutcome).
    virtual DamageOutcome receive_damage(int damage, Player& atacante, bool is_critical) = 0;

    virtual bool is_dead() const = 0;
};

#endif
