#ifndef DEFENSE_SET_H_
#define DEFENSE_SET_H_

#include "item/defense_item.h"

// Conjunto de equipamiento defensivo de un jugador: un slot de armadura, uno
// de casco y uno de escudo. NO es dueño de los items (el Inventory lo es): solo
// referencia cuáles están equipados y sabe calcular la defensa total.
class DefenseSet {
private:
    DefenseItem* armadura = nullptr;
    DefenseItem* casco = nullptr;
    DefenseItem* escudo = nullptr;

public:
    DefenseSet() = default;

    // Equipa en el slot correspondiente (solo referencia, no toma ownership).
    void equip_armadura(DefenseItem* item);
    void equip_casco(DefenseItem* item);
    void equip_escudo(DefenseItem* item);

    // Defensa = defensa(armadura) + defensa(casco) + defensa(escudo).
    // Los slots vacíos aportan 0.
    int calculate_defense() const;

    // Desequipa todos los slots (p. ej. al morir, cuando los items caen).
    void clear();
};

#endif
