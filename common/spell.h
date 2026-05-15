#ifndef SPELL_H
#define SPELL_H

class Player;

enum class SpellType {
    HEAL,      
    MISSILE,      
    EXPLOSION,  
};

class Spell {
private:
    SpellType type;

    void heal(int target_id);
    void missile(int target_id);
    void explosion(int target_id);

public:
    explicit Spell(SpellType type);

    void cast(int target_id);

    SpellType getType() const;
};

#endif