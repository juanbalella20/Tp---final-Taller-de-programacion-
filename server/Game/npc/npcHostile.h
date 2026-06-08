#ifndef NPC_HOSTILE_H_
#define NPC_HOSTILE_H_

#include <memory>

#include "npc.h"
#include "../entity.h"
// #include "itemDataBase.h"

class Player;
class Item;
struct Command;

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

        void death();
        void set_state(State state);
        // Oro BASE que deja caer el NPC al morir (cae siempre, sección "Oro"):
        // Oro = rand(0, npc_gold_drop_max) * VidaMaxNPC
        int drop();
        // Oro EXTRA del drop según factor: rand(min, max) * VidaMaxNPC.
        int roll_extra_gold() const;

    public:
        NPChostile(const std::string& type_id, const std::string& name,
                   int lifepoints, int attack_dmg, int ticks_to_spawn);
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
        
        
        
        // attacks player
        //void interact(Player player, Command cmd) override;
};

#endif 
