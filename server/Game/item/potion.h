#ifndef POTION_H_
#define POTION_H_

#include <string>

#include "item.h"
#include "../entity.h"

// Potion: objeto consumible que restaura vida y/o maná de quien la usa, con
// efecto inmediato. La poción de vida cura heal_life de vida; la de maná,
// heal_mana de maná. Es uno de los objetos que un NPC puede dejar caer al
// morir (ver la tabla de drop en config.toml / NPChostile).
//
// No es equipable: ItemType::OTHER. Al usarla, cura al propio jugador y se
// considera consumida (el llamador la saca del inventario).
class Potion : public Item {
private:
    int heal_life;
    int heal_mana;

public:
    Potion(const std::string& id, const std::string& name, int price,
           int heal_life, int heal_mana);

    int get_heal_life() const;
    int get_heal_mana() const;

    ItemType get_type() const override { return ItemType::OTHER; }

    // Cura al propio jugador (heal_life de vida + heal_mana de maná, cada uno
    // topeado por su máximo). Ignora el target y la posición. Devuelve un
    // outcome vacío: tomar una poción no produce daño.
    DamageOutcome use_item(Entity& target, Player& atacante,
                           int attacker_x, int attacker_y,
                           int target_x, int target_y, bool is_critical) override;
};

#endif
