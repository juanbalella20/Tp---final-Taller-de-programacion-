#ifndef ENTITY_H_
#define ENTITY_H_

#include <string>

class Player;

class Entity {
public:
    virtual ~Entity() = default;

    virtual std::string get_name() const = 0;

    // El target recibe daño y, con sus propios atributos (nivel, vida máxima),
    // recompensa al atacante con la XP correspondiente. Devuelve el oro que
    // droppea al suelo (0 si no murió o no aplica).
    virtual int receive_damage(int damage, Player& atacante, bool is_critical) = 0;

    virtual bool is_dead() const = 0;
};

#endif
