#ifndef NPC_H_
#define NPC_H_

#include <string>

/*
struct Command {
    int action; // ex: comprar
    int objeto; // ex: un item
    int cantidad; //
    int actual_gold; 
}
*/

class NPC {
    private:
        int npc_id;
        std::string name;
    // an npc_id is generated and a name
    NPC();
    public:
        virtual void interact(int player_tag, Command cmd);
};

#endif 
