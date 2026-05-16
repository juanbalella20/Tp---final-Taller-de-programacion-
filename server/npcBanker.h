#ifndef NPC_FRIENDLY_BANKER_H_
#define NPC_FRIENDLY_BANKER_H_


#include "npcFriendly.h"
#include <map>
#include <vector>
class NPCbanker: public NPCfriendly {
    private:
        //<player_name, items_vector>
        std::map<std::string, std::vector<Item>> item_bank;
        //<player_name, gold>
        std::map<std::string, int> gold_bank;

        // gets item bank
        std::map<int, std::vector<Item>> get_item_bank();
        // gets gold bank
        std::map<int, int> get_gold_bank();
        // adds item bank
        void receive_item(Player player, Item item);
        // adds gold from player
        void receive_gold(Player player, int ammount);
        // transfers item to player and deletes it from bank
        void transfer_item(Player player, Item item);
        // transfers gold to player and subtracts it from bank
        void transfer_gold(Player player, int ammount);
    



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
