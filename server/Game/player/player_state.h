#ifndef PLAYER_STATE_H_
#define PLAYER_STATE_H_

class Player;
class Entity;
struct DamageOutcome;

// Patrón State: gobierna qué puede hacer un jugador según esté vivo, fantasma o
// meditando. Cada estado EJECUTA el comportamiento de la acción (no solo responde flags).
class PlayerState {
public:
    virtual ~PlayerState() = default;

    // Ataca a un target. Devuelve el resultado del golpe (daño, oro, esquive).
    virtual DamageOutcome attack(Player& self, Entity& target, int target_x, int target_y) = 0;

    // Recibe daño de un atacante. Devuelve el resultado del golpe (daño aplicado,
    // oro dropeado al morir, si esquivó). Struct vacío si el estado no recibe daño.
    virtual DamageOutcome receive_damage(Player& self, int damage, Player& attacker, bool is_critical) = 0;
    virtual DamageOutcome receive_npc_damage(Player& self, int damage, bool is_critical) = 0;

    // Resucita al jugador (solo tiene efecto si está muerto).
    virtual void revive(Player& self) = 0;

    virtual bool is_ghost() const = 0;

    // Indica si el jugador puede interactuar con el mundo (tomar oro/items,
    // comerciar, etc.). El fantasma no puede.
    virtual bool can_interact() const = 0;

    // Avance de tiempo (game loop). El estado decide qué pasa con el correr de
    // los segundos: MeditateState regenera maná y devuelve true (el game loop
    // notifica el maná); el resto no hace nada y devuelve false.
    virtual bool tick(Player& self, double seconds) { (void)self; (void)seconds; return false; }

    // Pide salir de meditación. Solo MeditateState transiciona a vivo; el resto
    // de los estados ignoran el pedido (no estaban meditando).
    virtual void stop_meditation(Player& self) { (void)self; }

    // Alterna meditación (comando /meditar). Cada estado sabe qué hacer:
    // MeditateState deja de meditar; los demás (si pueden) empiezan. Devuelve
    // true si tras el toggle el jugador queda meditando.
    virtual bool toggle_meditation(Player& self) = 0;
};

#endif
