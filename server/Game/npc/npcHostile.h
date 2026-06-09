#ifndef NPC_HOSTILE_H_
#define NPC_HOSTILE_H_

#include <memory>

#include "npc.h"
#include "levelHostile.h"
#include "../entity.h"
// #include "itemDataBase.h"

#include <vector>

class Player;
class Item;
struct Command;
class ZoneWorld;

enum class State { DEAD, ALIVE };



class NPChostile : public NPC, public Entity {
    private:
        std::string type_id;
        int lifepoints;
        int attack_dmg;
        State state;
        int coord_x;
        int coord_y;
        int remaining_ticks_to_spawn;
        int ticks_to_spawn;
        int max_lifepoints;
        LevelHostile level;

        // persecución a jugador
        std::string target_player;
        int attack_speed_ticks;
        int current_attack_cooldown;

        void death();
        void set_state(State state);
        // Oro BASE que deja caer el NPC al morir (cae siempre, sección "Oro"):
        // Oro = rand(0, npc_gold_drop_max) * VidaMaxNPC
        int drop();
        // Oro EXTRA del drop según factor: rand(min, max) * VidaMaxNPC.
        int roll_extra_gold() const;

    public:
        NPChostile(const std::string& type_id, const std::string& name,
                   int lifepoints, int attack_dmg, int ticks_to_spawn,
                   int level);
        std::string get_name() const override;
        const std::string& get_type_id() const;
        DamageOutcome receive_damage(int damage, Player& atacante, bool is_critical) override;
        bool is_dead() const override;
        bool can_spawn() const;
        void reduce_ticks_to_spawn();
        void revive(int x, int y);
        void set_position(int x, int y);
        int get_coord_x() const;
        int get_coord_y() const;

        // Lógica de persecución a jugador
        void set_target(const std::string& player_name) { target_player = player_name; }
        void clear_target() { target_player.clear(); }
        bool has_target() const { return !target_player.empty(); }
        const std::string& get_target() const { return target_player; }
        void move_towards(int target_x, int target_y, ZoneWorld& world, const std::vector<Player*>& players); // Mueve 1 tile hacia el objetivo
        
        // attacks player
        bool can_attack() const;
        void reset_attack_cooldown();
        void tick_cooldowns();
        int get_damage() const;
};

#endif 
