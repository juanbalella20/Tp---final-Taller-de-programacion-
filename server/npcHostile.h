#ifndef NPC_HOSTILE_H_
#define NPC_HOSTILE_H_

#include "npc.h"
#include <vector>

class NPChostile : public NPC {
    private:
        // (x,y)
        std::vector<int> min_attack_distance;
        int type;
        int hp;
        int attack;

        NPChostile();
    public:
        // stats of each NPC depedns on creature and type
        NPChostile set_goblin(int type);
        NPChostile set_skeleton(int type);
        NPChostile set_zombie();
        NPChostile set_spider(int type);
        NPChostile set_orc();
        NPChostile set_golem(int type);
        
        // attacks player
        void interact(Player player_tag) override;
        // drops (or not) an item when slained 
        Item drop_item();
};

#endif 
