#ifndef LEVEL_HOSTILE_H_
#define LEVEL_HOSTILE_H_

// Nivel de una criatura hostil. A mayor nivel, más potente el NPC: cada NPC
// puede ser más fuerte que otro. El nivel escala linealmente la vida y el daño
// base del NPC (un golem de mazmorra es "mucho más fuerte que el promedio").
class LevelHostile {
private:
    int level;

public:
    explicit LevelHostile(int level);

    int get_level() const;

    // Daño efectivo del NPC = nivel * daño base.
    int calculateDamage(int damage) const;

    // Vida efectiva del NPC = nivel * vida base.
    int calculateLife(int life) const;
};

#endif
