#ifndef ALIVE_STATE_H_
#define ALIVE_STATE_H_

#include "player_state.h"

// Estado de un jugador vivo: puede atacar, recibir daño e interactuar.
// Si el daño recibido lo mata, transiciona al estado fantasma.
class AliveState : public PlayerState {
public:
    DamageOutcome attack(Player& self, Entity& target, int target_x, int target_y) override;
    DamageOutcome receive_damage(Player& self, int damage, Player& attacker, bool is_critical) override;
    DamageOutcome receive_npc_damage(Player& self, int damage, bool is_critical) override;
    void revive(Player& self) override;
    bool is_ghost() const override { return false; }
    bool can_interact() const override { return true; }
    bool toggle_meditation(Player& self) override;
};

#endif
