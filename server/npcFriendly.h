#ifndef NPC_FRIENDLY_H_
#define NPC_FRIENDLY_H_

#include "npc.h"

class NPCfriendly : public NPC {
    public:
        // an npc_id is generated and a name
        NPCfriendly();
        void interact(Player player, Command cmd) override;
        
};

#endif 
