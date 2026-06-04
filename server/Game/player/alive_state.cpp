#include "alive_state.h"
#include "player.h"
#include <cstdlib>
#include <cmath>
#include <algorithm>

int AliveState::attack(Player& self, Entity& target, int target_x, int target_y) {
    return self.player_inventory.use_equipped(target, self, self.coord_x, self.coord_y,
                                              target_x, target_y, false);
}

int AliveState::receive_damage(Player& self, int damage, Player& attacker, bool is_critical) {
    // Esquive: rand(0,1)^Agilidad < 0.001 (solo si no es crítico)
    if (!is_critical) {
        int agility = self.player_race.race_agility() + self.player_class.class_agility();
        double rnd = std::rand() / static_cast<double>(RAND_MAX);
        if (std::pow(rnd, agility) < 0.001) return 0;  // esquivó

        // Defensa: armadura + escudo + casco
        int defense = self.calculate_defense();
        damage = std::max(0, damage - defense);
    }

    bool murio = false;
    if (static_cast<int>(self.lives) <= damage) {
        self.lives = 0;
        murio = true;
    } else {
        self.lives -= static_cast<uint32_t>(damage);
    }

    attacker.ganar_xp(damage, self.level.get_number(), murio, static_cast<int>(self.max_life()));

    if (murio) {
        uint32_t drop = self.level.calculate_gold_drop(self.gold);
        self.gold -= drop;
        self.to_ghost();
        return static_cast<int>(drop);
    }
    return 0;
}

void AliveState::revive(Player& self) {
    // Ya está vivo: no hace nada.
    (void)self;
}
