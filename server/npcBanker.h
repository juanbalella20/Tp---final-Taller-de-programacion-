#ifndef NPC_FRIENDLY_BANKER_H_
#define NPC_FRIENDLY_BANKER_H_


#include "npcFriendly.h"
#include <map>
#include <vector>
class NPCbanker: public NPCfriendly {
    private:
        //<player_tag, items_vector>
        std::map<int, std::vector<Item>> item_bank;
        //<player_tag, gold>
        std::map<int, int> gold_bank;

        // gets item bank
        std::map<int, std::vector<Item>> get_item_bank();
        // gets gold bank
        std::map<int, int> get_gold_bank();
        // modifies item bank
        void update_item_bank();
        // modifies gold bank
        void update_gold_bank();
        
        // gives item from item bank to player
        Item withdraw_item();

        // gives gold from gold bank to player
        int withdraw_gold();

        // saves item from player
        void save_item();
        // saves gold from player
        void save_gold();


    public:
        // an npc_id is generated and a name
        NPCbanker();
        /* interacts with player:
         * - saves item from player
         * - gives item to player
         * - saves gold from player
         * - give gold to player
         */
        void interact(Player player_tag, Command cmd) override;

};

#endif 
