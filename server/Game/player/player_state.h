#ifndef PLAYER_STATE_H_
#define PLAYER_STATE_H_

class Player;
class Entity;

// Patrón State: gobierna qué puede hacer un jugador según esté vivo o fantasma.
// Cada estado EJECUTA el comportamiento de la acción (no solo responde flags).
class PlayerState {
public:
    virtual ~PlayerState() = default;

    // Ataca a un target. Devuelve el daño infligido.
    virtual int attack(Player& self, Entity& target, int target_x, int target_y) = 0;

    // Recibe daño de un atacante. Devuelve el oro que droppea al suelo (0 si no
    // murió o si el estado no recibe daño).
    virtual int receive_damage(Player& self, int damage, Player& attacker) = 0;

    // Resucita al jugador (solo tiene efecto si está muerto).
    virtual void revive(Player& self) = 0;

    virtual bool is_ghost() const = 0;

    // Indica si el jugador puede interactuar con el mundo (tomar oro/items,
    // comerciar, etc.). El fantasma no puede.
    virtual bool can_interact() const = 0;
};

#endif
