#ifndef GHOST_STATE_H_
#define GHOST_STATE_H_

#include "player_state.h"

// Estado de un jugador muerto (fantasma): no puede atacar, no recibe daño ni
// interactúa. Solo puede resucitar. (Moverse no pasa por el estado: el fantasma
// se mueve libremente.)
class GhostState : public PlayerState {
public:
    int attack(Player& self, Entity& target, int target_x, int target_y) override;
    int receive_damage(Player& self, int damage, Player& attacker, bool is_critical) override;
    void revive(Player& self) override;
    bool is_ghost() const override { return true; }
    bool can_interact() const override { return false; }
};

#endif
