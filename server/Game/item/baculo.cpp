#include "baculo.h"
#include "../entity.h"
#include "../player/player.h"
#include "../../game_exceptions.h"
#include "game_config.h"
#include <cmath>
#include <cstdlib>

Baculo::Baculo(const std::string& id, const std::string& name, int price, SpellKind kind,
               int mana_cost, int distance_min_attack, int min_value, int max_value):
        Item(id, name, price), kind(kind), mana_cost(mana_cost),
        distance_min_attack(distance_min_attack), min_value(min_value), max_value(max_value) {}

int Baculo::get_mana_cost() const {
    return mana_cost;
}

DamageOutcome Baculo::use_item(Entity& target, Player& atacante, int attacker_x, int attacker_y,
                     int target_x, int target_y, bool is_critical) {
    // El guerrero no puede usar magia.
    if (!atacante.can_cast()) throw CannotCastException();

    // Cada hechizo requiere un mínimo de maná y consume esa cantidad.
    if (static_cast<int>(atacante.get_mana()) < mana_cost) throw NotEnoughManaException();

    int rango = max_value - min_value;
    int roll = (rango > 0) ? (std::rand() % (rango + 1)) : 0;
    int value = min_value + roll;

    if (kind == SpellKind::CURAR) {
        // El hechizo de curación restaura vida sobre el propio lanzador.
        atacante.lose_mana(mana_cost);
        atacante.heal_life(value);
        return {};
    }

    // Hechizo de daño a distancia: como un arco, click sobre el target.
    int dx = std::abs(attacker_x - target_x);
    int dy = std::abs(attacker_y - target_y);
    if (dx + dy > distance_min_attack) throw OutOfRangeException();

    atacante.lose_mana(mana_cost);

    // Crítico: probabilidad cfg.crit_chance, daño x cfg.crit_multiplier, no esquivable.
    int damage = value;
    const auto& cfg = GameConfig::instance();
    bool critical = is_critical || ((std::rand() / static_cast<double>(RAND_MAX)) < cfg.crit_chance);
    if (critical) damage *= cfg.crit_multiplier;

    return target.receive_damage(damage, atacante, critical);
}
