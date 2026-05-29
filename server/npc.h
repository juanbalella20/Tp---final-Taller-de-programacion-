#ifndef NPC_H_
#define NPC_H_

#include <string>

enum SellerAction {
    ACTION_SELL = 0,
    ACTION_BUY  = 1,
    ACTION_LIST = 2,
};


struct Command {
    int action;       // ex: comprar, vender
    std::string item_id;  // id del item involucrado
    int cantidad;
    int actual_gold;
};

class NPC {
    protected:
        int npc_id;
        std::string name;
        //ItemDataBase itemdatabase;
    public:
    // an npc_id is generated and a name
    NPC();
    public:
        static void set_npc_id(int npc_id);
        static void set_npc_name(std::string npc_name);
};  

#endif 
