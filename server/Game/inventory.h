#ifndef INVENTORY_H_
#define INVENTORY_H_

#include <vector>
#include <memory>
#include <string>
#include <cstdint>

#include "item/item.h"
#include "entity.h"

class Player;

class Inventory {

private:
    std::vector<std::unique_ptr<Item>> items;
    Item* equipped_item = nullptr;

    static constexpr int MAX_SLOTS = 25;  //a definir

public:
    Inventory() {}

    std::vector<Item*> get_items() const;

    bool add_item(std::unique_ptr<Item> item);

    std::unique_ptr<Item> drop_item(Item* item);

    // Devuelve todos los items del inventario y lo deja vacío.
    std::vector<std::unique_ptr<Item>> drop_all();

    // Equipa por uid de INSTANCIA (distingue dos copias del mismo tipo). No-op
    // si el uid no está en el inventario o el item no ocupa el slot de arma.
    void equip_item(uint64_t item_uid);

    // Devuelve el item con ese uid de instancia (sin ceder ownership), o nullptr.
    Item* find_item(uint64_t item_uid) const;

    void unequip_item();

    // ¿El item con ese uid de instancia es el arma actualmente equipada?
    bool is_equipped(uint64_t item_uid) const;

    // ¿Hay un arma equipada?
    bool has_weapon_equipped() const;

    // uid de instancia del arma equipada, o 0 si no hay ninguna.
    uint64_t get_equipped_weapon_uid() const;

    // type_id del arma equipada, o "" si no hay ninguna (para el sprite/cliente).
    std::string get_equipped_weapon_type_id() const;

    bool is_full() const;
    DamageOutcome use_equipped(Entity& target, Player& atacante, int attacker_x, int attacker_y, int target_x, int target_y, bool is_critical);

    // Consume el item del inventario con ese uid de instancia (p.ej. una poción):
    // aplica su efecto sobre 'self' (use_item) y lo elimina del inventario.
    // Solo se consumen items no equipables (ItemType::OTHER). Devuelve true si se
    // consumió; false si el uid no existe o el item no es consumible.
    bool use_consumable(uint64_t item_uid, Player& self);
};


#endif