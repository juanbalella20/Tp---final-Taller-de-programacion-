#include "arma.h"
#include "../entity.h"
#include "../player/player.h"
#include <cmath>
#include <cstdlib>
#include "game_config.h"

Arma::Arma(const std::string& id, const std::string& name, int price, int disntance_min_attack,
           int damage_min, int damage_max):
        Item(id, name, price), disntance_min_attack(disntance_min_attack),
        damage_min(damage_min), damage_max(damage_max) {}

int Arma::get_disntance_min_attack() const {
    return disntance_min_attack;
}

DamageOutcome Arma::use_item(Entity& target, Player& atacante, int attacker_x, int attacker_y, int target_x, int target_y, bool is_critical) {
    int dx = std::abs(attacker_x - target_x);
    int dy = std::abs(attacker_y - target_y);
    if (dx + dy > disntance_min_attack) return {};

    // Daño = Fuerza * rand(DañoArmaMin, DañoArmaMax)
    int rango = damage_max - damage_min;
    int roll = (rango > 0) ? (std::rand() % (rango + 1)) : 0;
    int damage = atacante.damage_attack() * (damage_min + roll);

    // Crítico: probabilidad cfg.crit_chance, daño x cfg.crit_multiplier, no esquivable.
    const auto& cfg = GameConfig::instance();
    bool critical = is_critical || ((std::rand() / static_cast<double>(RAND_MAX)) < cfg.crit_chance);
    if (critical) damage *= cfg.crit_multiplier;

    return target.receive_damage(damage, atacante, critical);
}
