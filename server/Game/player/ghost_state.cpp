#include "ghost_state.h"
#include "player.h"
#include "../../game_exceptions.h"

int GhostState::attack(Player& self, Entity& target, int target_x, int target_y) {
    (void)self; (void)target; (void)target_x; (void)target_y;
    throw AttackNotAllowedException("Estás muerto, no podés atacar");
}

int GhostState::receive_damage(Player& self, int damage, Player& attacker, bool is_critical) {
    // Un fantasma no recibe daño ni reparte XP.
    (void)self; (void)damage; (void)attacker; (void)is_critical;
    return 0;
}

void GhostState::revive(Player& self) {
    self.lives = self.max_life();
    self.mana = self.max_mana();
    self.experience = 0;
    self.to_alive();
}
