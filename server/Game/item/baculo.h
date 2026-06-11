#ifndef BACULO_H
#define BACULO_H

#include <string>

#include "item.h"
#include "../entity.h"

// Tipo de hechizo que lanza un báculo/vara.
// DANIO: hechizo de ataque a distancia (flecha mágica, misil, explosión).
// CURAR: restaura vida sobre el propio lanzador (flauta élfica).
enum class SpellKind {
    DANIO,
    CURAR,
};

// Vara o báculo: item equipable que lanza un hechizo consumiendo maná.
// Se equipa en el mismo slot que el arma (no se puede tener arma y báculo a la
// vez). Solo las clases que pueden usar magia (todas salvo el guerrero) pueden
// usarlo. El ataque es a distancia, como un arco.
class Baculo : public Item {
private:
    SpellKind kind;
    int mana_cost;
    int distance_min_attack;
    // Para DANIO: rango de daño. Para CURAR: rango de vida restaurada.
    int min_value;
    int max_value;

public:
    Baculo(const std::string& id, const std::string& name, int price, SpellKind kind,
           int mana_cost, int distance_min_attack, int min_value, int max_value);

    int get_mana_cost() const;

    ItemType get_type() const override { return ItemType::MAGIC; }
    bool can_target_self() const override { return kind == SpellKind::CURAR; }

    // Lanza el hechizo. Si el atacante no puede usar magia lanza
    // CannotCastException; si no le alcanza el maná, NotEnoughManaException.
    // Para CURAR: cura al propio atacante y devuelve un outcome vacío. Para DANIO:
    // aplica daño a distancia sobre el target y devuelve el resultado del golpe.
    DamageOutcome use_item(Entity& target, Player& atacante, int attacker_x, int attacker_y,
                 int target_x, int target_y, bool is_critical) override;
};

#endif
