#include "armour.h"
#include "player.h"

Armour::Armour(ArmourType type)
    : Item(0, "", 0), type(type), defense_min(0), defense_max(0) {}

void Armour::update_player_defense(Player& player) {
    // aplica la defensa de esta pieza al player
}

void Armour::use_item(Player& player) {
    player.equip_item(getId());
}

ArmourType Armour::getType() const { 
    return type; 
}

int Armour::getDefenseMin() const { 
    return defense_min; 
}

int Armour::getDefenseMax() const { 
    return defense_max; 
}