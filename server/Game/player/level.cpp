#include "level.h"
#include <cmath>
#include <cstdlib>
#include <algorithm>
#include "game_config.h"

Level::Level(int number) : number(number) {}

int Level::get_number() const {
    return number;
}

bool Level::try_level_up(uint32_t experience) {
    const auto& cfg = GameConfig::instance();
    int limit = static_cast<int>(cfg.xp_base * std::pow(number, cfg.xp_exponent));
    if (experience >= static_cast<uint32_t>(limit)) {
        number += 1;
        return true;
    }
    return false;
}

uint32_t Level::max_gold() const {
    const auto& cfg = GameConfig::instance();
    return static_cast<uint32_t>(cfg.gold_base * std::pow(number, cfg.gold_exponent));
}

uint32_t Level::calculate_gold_drop(uint32_t gold) const {
    uint32_t max = max_gold();
    if (gold <= max) return 0;
    return gold - max;
}

int Level::xp_per_attack(int damage, int target_level) const {
    const auto& cfg = GameConfig::instance();
    int factor = std::max(target_level - number + cfg.xp_level_offset, 0);
    return damage * factor;
}

int Level::xp_per_kill(int target_max_life, int target_level) const {
    const auto& cfg = GameConfig::instance();
    int factor = std::max(target_level - number + cfg.xp_level_offset, 0);
    double rnd = (std::rand() / static_cast<double>(RAND_MAX)) * cfg.xp_kill_factor;
    return static_cast<int>(rnd * target_max_life * factor);
}

bool Level::is_newbie() const {
    const auto& cfg = GameConfig::instance();
    return number <= cfg.newbie_max_level;
}

bool Level::can_attack_level(int other_level) const {
    const auto& cfg = GameConfig::instance();
    return std::abs(number - other_level) <= cfg.level_diff_max;
}
