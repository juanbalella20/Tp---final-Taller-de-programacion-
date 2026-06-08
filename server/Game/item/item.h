#ifndef ITEM_H
#define ITEM_H

#include <string>
#include <cstdint>

#include "../entity.h"  // DamageOutcome (retorno de use_item)

class Player;

// Tipo de item, usado para que el cliente sepa en qué slot del HUD resaltarlo.
// Los valores se serializan en el protocolo: no reordenar.
enum class ItemType : uint8_t {
    WEAPON = 0,
    ARMOR  = 1,
    HELMET = 2,
    SHIELD = 3,
    OTHER  = 4,
    MAGIC  = 5,  // varas/báculos: lanzan hechizos consumiendo maná.
};

class Item {
protected:
    // id de TIPO: compartido por todas las instancias del mismo item (p.ej.
    // "espada"). Identifica al item en el catalogo y resuelve su imagen/stats.
    std::string id;
    std::string name;
    int price;
    // id de INSTANCIA: unico por objeto creado. Distingue dos copias del mismo
    // tipo (dos espadas) para equipar/resaltar la correcta. Se asigna en el ctor
    // con un contador global y NO se persiste (se regenera al recrear el item).
    uint64_t uid;

    static uint64_t next_uid;

public:
    Item(const std::string& id, const std::string& name, int price);
    virtual ~Item() = default;

    std::string get_id() const;

    // id de instancia unico (ver 'uid' arriba).
    uint64_t get_uid() const;


    std::string getName() const;

    int getPrice() const;

    // Tipo del item (para que el cliente lo ubique en el slot correcto del HUD).
    virtual ItemType get_type() const = 0;

    virtual DamageOutcome use_item(Entity& target, Player& atacante, int attacker_x, int attacker_y, int target_x, int target_y, bool is_critical) = 0;
};

#endif