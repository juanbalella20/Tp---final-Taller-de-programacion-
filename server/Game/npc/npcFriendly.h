#ifndef NPC_FRIENDLY_H_
#define NPC_FRIENDLY_H_

#include "npc.h"

class Player;

class NPCfriendly : public NPC {
    public:
        NPCfriendly() : NPC() {}
        virtual void interact(Player& player, Command cmd) = 0;
        virtual ~NPCfriendly() = default;
};

#endif