#ifndef NPC_FRIENDLY_H_
#define NPC_FRIENDLY_H_

#include "npc.h"

class NPCfriendly : public NPC {

    // an npc_id is generated and a name
    NPCfriendly(int npc_id, std::string name);
    public:
        void interact(int player_tag, Command cmd) override;
};

#endif 
