#include "ghost_state.h"
#include "player.h"
#include "../../game_exceptions.h"
#include "../../common/constants/game_config.h"
#include <algorithm>

DamageOutcome GhostState::attack(Player& self, Entity& target, int target_x, int target_y) {
    (void)self; (void)target; (void)target_x; (void)target_y;
    throw AttackNotAllowedException("Estás muerto, no podés atacar");
}

DamageOutcome GhostState::receive_damage(Player& self, int damage, Player& attacker, bool is_critical) {
    // Un fantasma no recibe daño ni reparte XP.
    (void)self; (void)damage; (void)attacker; (void)is_critical;
    return {};
}

DamageOutcome GhostState::receive_npc_damage(Player& self, int damage, bool is_critical) {
    // Un fantasma no recibe daño ni reparte XP.
    (void)self; (void)damage; (void)is_critical;
    return {};
}

void GhostState::revive(Player& self) {
    const GameConfig& cfg = GameConfig::instance();
    self.lives = std::max(1, static_cast<int>(self.max_life() * cfg.revive_life_factor));
    self.mana  = std::max(0, static_cast<int>(self.max_mana() * cfg.revive_life_factor));
    self.to_alive();
}