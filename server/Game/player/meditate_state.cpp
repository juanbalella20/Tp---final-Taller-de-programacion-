#include "meditate_state.h"
#include "player.h"

DamageOutcome MeditateState::attack(Player& self, Entity& target, int target_x, int target_y) {
    self.to_alive();
    return self.attack(target, target_x, target_y);
}

DamageOutcome MeditateState::receive_damage(Player& self, int damage, Player& attacker, bool is_critical) {
    self.to_alive();
    return self.receive_damage(damage, attacker, is_critical);
}

DamageOutcome MeditateState::receive_npc_damage(Player& self, int damage, bool is_critical) {
    self.to_alive();
    return self.receive_npc_damage(damage, is_critical);
}

void MeditateState::revive(Player& self) {
    (void)self;
}

bool MeditateState::tick(Player& self, double seconds) {
    self.recover_meditation_mana(seconds);
    return true;
}

void MeditateState::stop_meditation(Player& self) {
    self.to_alive();
}

bool MeditateState::toggle_meditation(Player& self) {
    self.to_alive();
    return false;
}
