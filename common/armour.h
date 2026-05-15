#ifndef ARMOUR_H
#define ARMOUR_H

#include "item.h"

enum class ArmourType {
    LEATHER_ARMOR, PLATE_ARMOR, BLUE_TUNIC,  
    HOOD, IRON_HELMET, MAGIC_HAT,            
    TURTLE_SHIELD, IRON_SHIELD               
};

class Player;

class Armour : public Item {
private:
    ArmourType type;
    int defense_min;
    int defense_max;

public:
    explicit Armour(ArmourType type);

    void update_player_defense(Player& player);
    void use_item(Player& player) override;

    ArmourType getType() const;
    int getDefenseMin() const;
    int getDefenseMax() const;
};

#endif