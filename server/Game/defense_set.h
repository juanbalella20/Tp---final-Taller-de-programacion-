#ifndef DEFENSE_SET_H_
#define DEFENSE_SET_H_

#include <cstdint>
#include <string>
#include <vector>

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

    // Desequipa el slot correspondiente.
    void unequip_armadura();
    void unequip_casco();
    void unequip_escudo();

    // ¿El item con ese uid de instancia está equipado en algún slot de defensa?
    bool is_equipped(uint64_t item_uid) const;

    // uids de instancia de los items equipados (armadura, casco, escudo); omite
    // slots vacíos.
    std::vector<uint64_t> get_equipped_uids() const;

    // type_ids de los items equipados (para el sprite/animaciones del cliente).
    std::vector<std::string> get_equipped_type_ids() const;

    // Defensa = defensa(armadura) + defensa(casco) + defensa(escudo).
    // Los slots vacíos aportan 0.
    int calculate_defense() const;

    // Desequipa todos los slots (p. ej. al morir, cuando los items caen).
    void clear();
};

#endif
