#ifndef NPC_FRIENDLY_PRIEST_H_
#define NPC_FRIENDLY_PRIEST_H_

#include "npcFriendly.h"

class NPCpriest : public NPCfriendly {
    private:
        // store can be a class
        Store store;

        // gets priest store
        Store get_store();
        // adds or delete an item from the store
        void update_store();
        // revives player
        void revive(int player_tag);
        // heals player
        void heal(int player_tag);
        // sells item to player
        Item sell_item(int item_id, int player_tag, int gold, int cantidad);

    public:
        // an npc_id is generated and a name
        NPCpriest();
        /* interacts with player:
         * - reseructs player
         * - heals player
         * - sells item to player
         */
        void interact(int player_tag, Command cmd) override;

};

#endif 
