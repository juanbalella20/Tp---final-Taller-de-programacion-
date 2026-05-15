#ifndef MAGIC_ITEM_H
#define MAGIC_ITEM_H

#include "item.h"

enum class MagicItemType {
    ASH_STAFF,     
    ELVEN_FLUTE,    
    KNOTTY_STAFF,   
    CRYSTAL_STAFF,  
};

class Player;

class MagicItem : public Item {
private:
    MagicItemType type;
    int damage_min;
    int damage_max;
    int mana_cost;
    bool ranged;

public:
    explicit MagicItem(MagicItemType type);

    void cast_spell(int spell_id, Player& target);
    void use_item(Player& player) override;

    MagicItemType getType()      const;
    int getDamageMin()           const;
    int getDamageMax()           const;
    int getManaCost()            const;
    bool isRanged()              const;
};

#endif