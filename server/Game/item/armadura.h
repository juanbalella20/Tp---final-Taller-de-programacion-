#ifndef ARMADURA_H_
#define ARMADURA_H_

#include "defense_item.h"

// Armadura: ocupa el slot de armadura del inventario.
// Ej: Armadura de cuero (2-6), Armadura de placas (15-30), Túnica azul (6-10).
class Armadura : public DefenseItem {
public:
    Armadura(const std::string& id, const std::string& name, int price,
             int defense_min, int defense_max);

    ItemType get_type() const override { return ItemType::ARMOR; }
};

#endif
