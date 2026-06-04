#include "alive_state.h"
#include "player.h"

int AliveState::attack(Player& self, Entity& target, int target_x, int target_y) {
    return self.player_inventory.use_equipped(target, self, self.coord_x, self.coord_y,
                                              target_x, target_y);
}

int AliveState::receive_damage(Player& self, int damage, Player& attacker) {
    bool murio = false;
    if (static_cast<int>(self.lives) <= damage) {
        self.lives = 0;
        murio = true;
    } else {
        self.lives -= static_cast<uint32_t>(damage);
    }

    // El target recompensa al atacante con sus propios atributos (nivel, vida máxima)
    attacker.ganar_xp(damage, self.level.get_number(), murio, static_cast<int>(self.max_life()));

    if (murio) {
        uint32_t drop = self.level.calculate_gold_drop(self.gold);
        self.gold -= drop;
        self.to_ghost();  // transiciona a fantasma
        return static_cast<int>(drop);
    }
    return 0;
}

void AliveState::revive(Player& self) {
    // Ya está vivo: no hace nada.
    (void)self;
}
