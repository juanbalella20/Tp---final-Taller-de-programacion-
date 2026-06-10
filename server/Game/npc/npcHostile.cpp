#include "npcHostile.h"
#include "../player/player.h"
#include "../zoneWorld.h"
#include <cstdlib>
#include "game_config.h"
#include "../item/item.h"
#include "../item/item_catalog.h"
#include <array>
#include <queue>
#include <utility>
#include <vector>

NPChostile::NPChostile(const std::string& type_id, const std::string& name,
                       int lifepoints, int attack_dmg, int ticks_to_spawn,
                       int level)
    : type_id(type_id),
      // Vida y daño base escalados por el nivel: un NPC de mayor nivel es más
      // potente que otro. LevelHostile centraliza la fórmula (nivel * base).
      lifepoints(LevelHostile(level).calculateLife(lifepoints)),
      attack_dmg(LevelHostile(level).calculateDamage(attack_dmg)),
      state(State::ALIVE), coord_x(0), coord_y(0),
      remaining_ticks_to_spawn(0), ticks_to_spawn(ticks_to_spawn),
      max_lifepoints(this->lifepoints),
      level(level),
      current_direction(DIR_SOUTH),
      // En ticks de 50ms (los decrementa ZoneWorld::update_npcs cada tick):
      // 20 ticks = 1 ataque por segundo.
      attack_speed_ticks(20),
      current_attack_cooldown(0) {
    this->name = name;
}

std::string NPChostile::get_name() const { return this->name; }
const std::string& NPChostile::get_type_id() const { return type_id; }

void NPChostile::set_position(int x, int y) { coord_x = x; coord_y = y; }
int NPChostile::get_coord_x() const { return coord_x; }
int NPChostile::get_coord_y() const { return coord_y; }

bool NPChostile::is_dead() const { return state == State::DEAD; }

bool NPChostile::can_spawn() const {
    return state == State::DEAD && remaining_ticks_to_spawn == 0;
}

void NPChostile::set_state(State s) { this->state = s; }

int NPChostile::drop() {
    // Oro base = rand(0, npc_gold_drop_max) * VidaMaxNPC
    const auto& cfg = GameConfig::instance();
    double factor = (std::rand() / static_cast<double>(RAND_MAX)) * cfg.npc_gold_drop_max;
    return static_cast<int>(factor * max_lifepoints);
}

int NPChostile::roll_extra_gold() const {
    // Oro extra de la tabla = rand(npc_gold_drop_min, npc_gold_drop_max) * VidaMaxNPC
    const auto& cfg = GameConfig::instance();
    double r = std::rand() / static_cast<double>(RAND_MAX);
    double factor = cfg.npc_gold_drop_min + r * (cfg.npc_gold_drop_max - cfg.npc_gold_drop_min);
    return static_cast<int>(factor * max_lifepoints);
}

void NPChostile::death() {
    lifepoints = 0;
    set_state(State::DEAD);
    remaining_ticks_to_spawn = ticks_to_spawn;
}

void NPChostile::reduce_ticks_to_spawn() {
    if (remaining_ticks_to_spawn > 0) {
        remaining_ticks_to_spawn--;
    }
}

void NPChostile::revive(int x, int y) {
    lifepoints = max_lifepoints;
    set_position(x, y);
    set_state(State::ALIVE);
}

DamageOutcome NPChostile::receive_damage(int dmg, Player& atacante, bool is_critical) {
    (void)is_critical;  // los NPC no esquivan ni tienen defensa modelada aún
    lifepoints -= dmg;
    bool murio = false;
    int gold_drop = 0;
    std::unique_ptr<Item> dropped_item;
    if (lifepoints <= 0) {
        // 1) Oro base: cae SIEMPRE al morir (sección "Oro" del enunciado).
        gold_drop = drop();
        // 2) Drop EXTRA: se sortea según la tabla [npcs.drop] (sección "Criaturas").
        //    Las franjas son acumuladas; el sobrante de probabilidad = "nada extra".
        const auto& cfg = GameConfig::instance();
        double r = std::rand() / static_cast<double>(RAND_MAX);
        double thr_gold   = cfg.npc_drop_prob_nothing + cfg.npc_drop_prob_gold;
        double thr_potion = thr_gold + cfg.npc_drop_prob_potion;
        double thr_item   = thr_potion + cfg.npc_drop_prob_item;
        if (r < cfg.npc_drop_prob_nothing) {
            // nada extra
        } else if (r < thr_gold) {
            gold_drop += roll_extra_gold();
        } else if (r < thr_potion) {
            ItemCatalog catalog;
            dropped_item = catalog.make_random_potion();
        } else if (r < thr_item) {
            ItemCatalog catalog;
            dropped_item = catalog.make_random_item();
        }
        death();
        murio = true;
    }
    // La XP se otorga SIEMPRE, haya muerto o no, antes de retornar el oro.
    bool level_up = atacante.ganar_xp(dmg, level.get_level(), murio, max_lifepoints);
    return {dmg, gold_drop, false, level_up, std::move(dropped_item)};
}

void NPChostile::move_towards(int target_x, int target_y, ZoneWorld& world, const std::vector<Player*>& players) {
    const int width = world.get_width();
    const int height = world.get_height();
    if (width <= 0 || height <= 0) return;

    const auto index_of = [width](int x, int y) { return y * width + x; };
    const auto x_of = [width](int index) { return index % width; };
    const auto y_of = [width](int index) { return index / width; };
    const auto is_adjacent_to_target = [target_x, target_y](int x, int y) {
        return std::abs(x - target_x) + std::abs(y - target_y) == 1;
    };
    if (is_adjacent_to_target(coord_x, coord_y)) return;

    const int start = index_of(coord_x, coord_y);
    std::vector<int> parent(width * height, -1);
    std::queue<int> pending;
    parent[start] = start;
    pending.push(start);

    int goal = -1;
    static constexpr std::array<std::pair<int, int>, 4> directions = {{
        {0, -1},
        {1, 0},
        {0, 1},
        {-1, 0},
    }};

    while (!pending.empty()) {
        const int current = pending.front();
        pending.pop();

        const int current_x = x_of(current);
        const int current_y = y_of(current);
        if (current != start && is_adjacent_to_target(current_x, current_y)) {
            goal = current;
            break;
        }

        for (const auto& [step_x, step_y] : directions) {
            const int next_x = current_x + step_x;
            const int next_y = current_y + step_y;
            if (!world.in_bounds(next_x, next_y)) continue;

            const int next = index_of(next_x, next_y);
            if (parent[next] != -1) continue;
            if (world.is_blocked_terrain(next_x, next_y)) continue;
            if (world.has_actor_at(next_x, next_y, players)) continue;

            parent[next] = current;
            pending.push(next);
        }
    }

    if (goal == -1) return;

    int first_step = goal;
    while (parent[first_step] != start) {
        first_step = parent[first_step];
    }

    const int next_x = x_of(first_step);
    const int next_y = y_of(first_step);
    if (next_x > coord_x) {
        current_direction = DIR_EAST;
    } else if (next_x < coord_x) {
        current_direction = DIR_WEST;
    } else if (next_y > coord_y) {
        current_direction = DIR_SOUTH;
    } else {
        current_direction = DIR_NORTH;
    }

    coord_x = next_x;
    coord_y = next_y;
}

bool NPChostile::can_attack() const {
    // Puede atacar si su tiempo de espera llegó a cero
    return current_attack_cooldown <= 0;
}

void NPChostile::reset_attack_cooldown() {
    // Reinicia el contador a su velocidad base
    current_attack_cooldown = attack_speed_ticks;
}

void NPChostile::tick_cooldowns() {
    // Si el contador es mayor a cero, lo reducimos
    if (current_attack_cooldown > 0) {
        current_attack_cooldown--;
    }
}

int NPChostile::get_damage() const {
    return attack_dmg;
}

/*

void NPChostile::set_type(int t) { this->type = t; }
void NPChostile::set_attack(int atk) { this->attack_dmg = atk; }
int NPChostile::get_attack() { return attack_dmg; }

void NPChostile::set_goblin(int type) {
    set_type(type);
    lifepoints = 30;
    set_attack(5);
    this->name = "Goblin";
}

void NPChostile::set_skeleton(int type) {
    set_type(type);
    lifepoints = 25;
    set_attack(7);
    this->name = "Skeleton";
}

void NPChostile::set_spider(int type) {
    set_type(type);
    lifepoints = 20;
    set_attack(4);
    this->name = "Spider";
}

void NPChostile::set_golem(int type) {
    set_type(type);
    lifepoints = 80;
    set_attack(15);
    this->name = "Golem";
}

void NPChostile::set_zombie() {
    lifepoints = 40;
    set_attack(8);
    this->name = "Zombie";
}

void NPChostile::set_orc() {
    lifepoints = 60;
    set_attack(12);
    this->name = "Orc";
}




*/
