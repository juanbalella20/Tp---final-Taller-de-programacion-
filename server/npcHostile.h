#ifndef NPC_HOSTILE_H_
#define NPC_HOSTILE_H_

#include "npc.h"
#include "itemDataBase.h"

enum class State { DEAD, ALIVE };



class NPChostile : public NPC {
    private:
        int type;
        int hp;
        int attack;
        int max_hp;
        State state;
        ItemDataBase itemdatabase;

        NPChostile();
        void set_hp(int hp);
        int get_hp();
        void set_attack(int attack);
        int get_attack();
        void set_type(int type);
        void attack(Player player);
        void receive_dmg(int dmg);
        void death();
        void set_state(State state);
        // drops a potion
        Item drop_potion();
        // drops an item 
        Item drop_item();
        // drops gold
        int drop_gold();
        // evaluates what to drop: nothing, gold or item
        void drop();
        // todo
        void respawn();
        void set_max_hp(int hp);
        int get_max_hp();

    public:
        // stats of each NPC depedns on creature and type
        void set_goblin(int type);
        void set_skeleton(int type);
        void set_spider(int type);
        void set_golem(int type);
        void set_zombie();
        void set_orc();
        
        // attacks player
        void interact(Player player, Command cmd) override;
};

#endif 
