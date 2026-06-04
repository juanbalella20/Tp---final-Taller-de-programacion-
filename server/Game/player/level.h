#ifndef LEVEL_H_
#define LEVEL_H_

#include <cstdint>

class Level {
private:
    int number;

    // GoldMax = 100 * level^1.1
    uint32_t max_gold() const;

public:
    explicit Level(int number);

    int get_number() const;

    // Returns true if level increased.
    bool try_level_up(uint32_t experience);

    // Returns how much gold drops on death.
    // If gold <= GoldMax → 0 (nothing drops)
    // If gold >  GoldMax → gold - GoldMax (the excess drops)
    uint32_t calculate_gold_drop(uint32_t gold) const;

    // XP gained by this level (attacker) for hitting a target of target_level.
    // Exp = damage * max(target_level - number + 10, 0)
    int xp_per_attack(int damage, int target_level) const;

    // Additional XP gained by this level (attacker) for killing the target.
    // Exp = rand(0, 0.1) * target_max_life * max(target_level - number + 10, 0)
    int xp_per_kill(int target_max_life, int target_level) const;

    // Fair play: nivel 12 o menor es newbie, no puede atacar ni ser atacado
    // por otros jugadores.
    bool is_newbie() const;

    // Fair play: no se puede atacar a otro jugador si la diferencia de niveles
    // supera 10.
    bool can_attack_level(int other_level) const;
};

#endif
