#include "level.h"
#include <cmath>
#include <cstdlib>
#include <algorithm>

Level::Level(int number) : number(number) {}

int Level::get_number() const {
    return number;
}

bool Level::try_level_up(uint32_t experience) {
    uint32_t limit = max_xp();
    if (experience >= limit) {
        number += 1;
        return true;
    }
    return false;
}

uint32_t Level::max_gold() const {
    return static_cast<uint32_t>(100 * std::pow(number, 1.1));
}

uint32_t Level::max_xp() const {
    return static_cast<uint32_t>(1000 * std::pow(number, 1.8));
}

uint32_t Level::calculate_gold_drop(uint32_t gold) const {
    uint32_t max = max_gold();
    if (gold <= max) return 0;
    return gold - max;
}

int Level::xp_per_attack(int damage, int target_level) const {
    int factor = std::max(target_level - number + 10, 0);
    return damage * factor;
}

int Level::xp_per_kill(int target_max_life, int target_level) const {
    int factor = std::max(target_level - number + 10, 0);
    double rnd = (std::rand() / static_cast<double>(RAND_MAX)) * 0.1;
    return static_cast<int>(rnd * target_max_life * factor);
}

bool Level::is_newbie() const {
    return number <= 12;
}

bool Level::can_attack_level(int other_level) const {
    return std::abs(number - other_level) <= 10;
}
