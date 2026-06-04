#ifndef DEFENSE_ITEM_H_
#define DEFENSE_ITEM_H_

#include "item.h"

// Clase base para items de defensa (armadura, casco, escudo).
// Cada subclase ocupa un slot distinto en el inventario.
// Para un item de defensa, use_item devuelve la defensa aportada:
// rand(defense_min, defense_max). Ignora los parámetros de ataque.
class DefenseItem : public Item {
protected:
    int defense_min;
    int defense_max;

public:
    DefenseItem(const std::string& id, const std::string& name, int price,
                int defense_min, int defense_max);

    // Defensa que aporta este item: rand(defense_min, defense_max).
    int calculate_defense() const;

    // "Usar" un item de defensa devuelve la defensa que aporta (ignora los
    // parámetros de ataque).
    int use_item(Entity& target, Player& atacante,
                 int attacker_x, int attacker_y,
                 int target_x, int target_y, bool is_critical) override;
};

#endif
