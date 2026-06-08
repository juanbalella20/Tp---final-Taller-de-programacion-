#ifndef ENTITY_H_
#define ENTITY_H_

#include <memory>
#include <string>

class Player;
class Item;

// Resultado de un golpe sobre un target. Desambigua los casos que antes se
// mezclaban en un solo int de retorno:
//   - damage: daño efectivo aplicado (post-defensa). 0 si esquivó o no aplicó.
//   - gold_drop: oro que el target droppea al suelo al morir (0 si no murió).
//     Para un NPC incluye el oro base + el oro extra de la tabla de drop.
//   - dropped_item: objeto que el NPC deja caer al morir según la tabla de drop
//     (poción o item al azar), o nullptr si no dropeó ninguno. Solo NPCs.
//   - dodged: el target esquivó el ataque (solo Player no-crítico).
// Move-only por dropped_item: se retorna por valor (move), nunca se copia.
struct DamageOutcome {
    int  damage = 0;
    int  gold_drop = 0;
    bool dodged = false;
    bool level_up = false;
    std::unique_ptr<Item> dropped_item = nullptr;
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
