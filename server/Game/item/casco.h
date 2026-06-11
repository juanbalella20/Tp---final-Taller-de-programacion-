#ifndef CASCO_H_
#define CASCO_H_

#include "defense_item.h"

// Casco: ocupa el slot de casco del inventario.
// Ej: Capucha (1-4), Casco de hierro (4-8), Sombrero mágico (4-12).
class Casco : public DefenseItem {
public:
    Casco(const std::string& id, const std::string& name, int price,
          int defense_min, int defense_max);

    ItemType get_type() const override { return ItemType::HELMET; }
};

#endif
