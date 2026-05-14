#ifndef NPC_H_
#define NPC_H_

#include <string>

/*
struct Command {
    int action; // ex: comprar
    int objeto; // ex: un item
    int cantidad; //
    int actual_gold; // ?
}
*/

class NPC {
    private:
        int npc_id;
        std::string name;
    // an npc_id is generated and a name
    NPC();
    public:
        virtual void interact(Player player, Command cmd);
        // se busca dentro de los npcs un numero y se setea
        static void set_npc_id(int npc_id);
        // se crea un nombre para el npc
        static void set_npc_name(std::string npc_name);
};  

#endif 
