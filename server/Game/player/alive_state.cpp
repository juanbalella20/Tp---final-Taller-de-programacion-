#include "alive_state.h"
#include "player.h"
#include <cstdlib>
#include <cmath>
#include <algorithm>

DamageOutcome AliveState::attack(Player& self, Entity& target, int target_x, int target_y) {
    return self.player_inventory.use_equipped(target, self, self.coord_x, self.coord_y,
                                              target_x, target_y, false);
}

DamageOutcome AliveState::receive_damage(Player& self, int damage, Player& attacker, bool is_critical) {
    // Esquive: rand(0,1)^Agilidad < 0.001 (solo si no es crítico)
    if (!is_critical) {
        int agility = self.player_race.race_agility() + self.player_class.class_agility();
        double rnd = std::rand() / static_cast<double>(RAND_MAX);
        if (std::pow(rnd, agility) < 0.001) return {0, 0, true};  // esquivó

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

    bool level_up =attacker.ganar_xp(damage, self.level.get_number(), murio, static_cast<int>(self.max_life()));

    if (murio) {
        uint32_t drop = self.level.calculate_gold_drop(self.gold);
        self.gold -= drop;
        self.to_ghost();
        return {damage, static_cast<int>(drop), false, level_up};
    }
    return {damage, 0, false, level_up};
}

DamageOutcome AliveState::receive_npc_damage(Player& self, int damage, bool is_critical) {
    if (!is_critical) {
        int agility = self.player_race.race_agility() + self.player_class.class_agility();
        double rnd = std::rand() / static_cast<double>(RAND_MAX);
        if (std::pow(rnd, agility) < 0.001) return {0, 0, true};  // esquivó

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

    if (murio) {
        uint32_t drop = self.level.calculate_gold_drop(self.gold);
        self.gold -= drop;
        self.to_ghost();
        return {damage, static_cast<int>(drop), false, false};
    }

    return {damage, 0, false, false};
}

void AliveState::revive(Player& self) {
    // Ya está vivo: no hace nada.
    (void)self;
}

bool AliveState::toggle_meditation(Player& self) {
    // Vivo y quieto: /meditar empieza la meditación si la clase lo permite.
    if (!self.can_meditate()) return false;
    self.to_meditate();
    return true;
}
