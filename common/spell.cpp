#include "spell.h"
#include "player.h"

Spell::Spell(SpellType type) : type(type) {}

void Spell::cast(int target_id) {
    switch (type) {
        case SpellType::HEAL:
            heal(target_id);
            break;
        case SpellType::MISSILE:
            missile(target_id);
            break;
        case SpellType::EXPLOSION:
            explosion(target_id);
            break;
    }
}

void Spell::heal(int target_id) {
    // restaura vida al jugador que lanza
    // consume 100 de mana
}

void Spell::missile(int target_id) {
    // daño a distancia 4-8
    // consume 15 de mana
}

void Spell::explosion(int target_id) {
    // daño a distancia 8-20
    // consume 30 de mana
}

SpellType Spell::getType() const { 
    return type; 
}
