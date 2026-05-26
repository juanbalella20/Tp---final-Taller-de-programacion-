#ifndef NPC_H_
#define NPC_H_

#include <string>


struct Command {
    int action; // ex: comprar
    int objeto; // ex: un item
    int cantidad; //
    int actual_gold; // ?
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
