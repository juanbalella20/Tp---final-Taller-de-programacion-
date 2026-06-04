#ifndef ITEM_H
#define ITEM_H

#include <string>
#include <cstdint>

class Entity;
class Player;

// Tipo de item, usado para que el cliente sepa en qué slot del HUD resaltarlo.
// Los valores se serializan en el protocolo: no reordenar.
enum class ItemType : uint8_t {
    WEAPON = 0,
    ARMOR  = 1,
    HELMET = 2,
    SHIELD = 3,
    OTHER  = 4,
};

class Item {
protected:
    std::string id;
    std::string name;
    int price;

public:
    Item(const std::string& id, const std::string& name, int price);
    virtual ~Item() = default;

    std::string get_id() const;


    std::string getName() const;

    int getPrice() const;

    // Tipo del item (para que el cliente lo ubique en el slot correcto del HUD).
    virtual ItemType get_type() const = 0;

    virtual int use_item(Entity& target, Player& atacante, int attacker_x, int attacker_y, int target_x, int target_y, bool is_critical) = 0;
};

#endif