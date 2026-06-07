#ifndef MEDITATE_STATE_H_
#define MEDITATE_STATE_H_

#include "player_state.h"

// Estado de un jugador que medita para recuperar maná. Sigue estando vivo: por
// eso CUALQUIER acción (atacar, recibir daño, dejar de meditar) lo devuelve al
// estado vivo y delega el comportamiento en él (no duplica la lógica de combate).
// El maná se recupera en el tick según FClaseMeditacion * Inteligencia * segundos.
class MeditateState : public PlayerState {
public:
    DamageOutcome attack(Player& self, Entity& target, int target_x, int target_y) override;
    DamageOutcome receive_damage(Player& self, int damage, Player& attacker, bool is_critical) override;
    void revive(Player& self) override;
    bool is_ghost() const override { return false; }
    bool can_interact() const override { return true; }
    bool tick(Player& self, double seconds) override;
    void stop_meditation(Player& self) override;
    bool toggle_meditation(Player& self) override;
};

#endif
