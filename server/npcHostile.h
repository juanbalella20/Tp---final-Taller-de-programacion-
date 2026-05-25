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
        int type;
        int lifepoints;
        int attack_dmg;
        State state;
        int coord_x;
        int coord_y;

        void set_attack(int attack);
        int get_attack();
        void set_type(int type);
        void death();
        void set_state(State state);
        void drop();

    public:
        NPChostile();
        std::string get_name() const override;
        void receive_damage(int damage) override;
        bool is_dead() const override;
        void set_position(int x, int y);
        int get_coord_x() const;
        int get_coord_y() const;

        // stats of each NPC depedns on creature and type
        void set_goblin(int type);
        void set_skeleton(int type);
        void set_spider(int type);
        void set_golem(int type);
        void set_zombie();
        void set_orc();
        
        // attacks player
        //void interact(Player player, Command cmd) override;
};

#endif 
