#include "magic_item.h"
#include "player.h"

MagicItem::MagicItem(MagicItemType type)
    : Item(0, "", 0), type(type)
    , damage_min(0), damage_max(0), mana_cost(0), ranged(true) {}

void MagicItem::cast_spell(int spell_id, Player& target) {
    // despacha el hechizo según spell_id
    // consume mana del player que lanza
}

void MagicItem::use_item(Player& player) {
    player.equip_item(getId());
}

MagicItemType MagicItem::getType() const { 
    return type; 
}

int MagicItem::getDamageMin() const { 
    return damage_min; 
}

int MagicItem::getDamageMax() const { 
    return damage_max; 
}

int MagicItem::getManaCost() const { 
    return mana_cost; 
}

bool MagicItem::isRanged() const { 
    return ranged; 
}
