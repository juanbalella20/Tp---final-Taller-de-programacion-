#include "levelHostile.h"

LevelHostile::LevelHostile(int level) : level(level) {}

int LevelHostile::get_level() const { return level; }

int LevelHostile::calculateDamage(int damage) const { return level * damage; }

int LevelHostile::calculateLife(int life) const { return level * life; }
