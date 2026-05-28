#ifndef NPC_HOSTILE_H_
#define NPC_HOSTILE_H_

#include "npc.h"
#include "entity.h"
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
        void drop();

    public:
        NPChostile(const std::string& type_id, const std::string& name,
                   int lifepoints, int attack_dmg, int ticks_to_spawn);
        std::string get_name() const override;
        const std::string& get_type_id() const;
        void receive_damage(int damage) override;
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
