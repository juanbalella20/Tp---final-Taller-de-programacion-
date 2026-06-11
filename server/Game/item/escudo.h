#ifndef ESCUDO_H_
#define ESCUDO_H_

#include "defense_item.h"

// Escudo: ocupa el slot de escudo del inventario.
// Ej: Escudo de tortuga (1-2), Escudo de hierro (1-4).
class Escudo : public DefenseItem {
public:
    Escudo(const std::string& id, const std::string& name, int price,
           int defense_min, int defense_max);

    ItemType get_type() const override { return ItemType::SHIELD; }
};

#endif
