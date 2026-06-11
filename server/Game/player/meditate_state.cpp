#include "meditate_state.h"
#include "player.h"

DamageOutcome MeditateState::attack(Player& self, Entity& target, int target_x, int target_y) {
    // Atacar saca de meditación: vuelve a vivo y delega en ese estado.
    self.to_alive();
    return self.attack(target, target_x, target_y);
}

DamageOutcome MeditateState::receive_damage(Player& self, int damage, Player& attacker, bool is_critical) {
    // Recibir daño saca de meditación: vuelve a vivo y delega en ese estado.
    self.to_alive();
    return self.receive_damage(damage, attacker, is_critical);
}

DamageOutcome MeditateState::receive_npc_damage(Player& self, int damage, bool is_critical) {
    // Recibir daño saca de meditación: vuelve a vivo y delega en ese estado.
    self.to_alive();
    return self.receive_npc_damage(damage, is_critical);
}

void MeditateState::revive(Player& self) {
    // Ya está vivo: no hace nada.
    (void)self;
}

bool MeditateState::tick(Player& self, double seconds) {
    self.recover_meditation_mana(seconds);
    return true;  // meditando: el game loop notifica el maná actualizado
}

void MeditateState::stop_meditation(Player& self) {
    self.to_alive();
}

bool MeditateState::toggle_meditation(Player& self) {
    // Ya estaba meditando: /meditar lo saca de meditación.
    self.to_alive();
    return false;
}
