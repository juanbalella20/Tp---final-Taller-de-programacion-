#include "clientGUI.h"
#include "../common/queue.h"
#include "../common/clientCmd.h"

class GameMsg {};  // stub until common/gameMsg.h exists

int main() {
    Queue<ClientCmd> outgoing;
    Queue<GameMsg> receiving;
    ClientGUI gui(outgoing, receiving);
    gui.run();
    return 0;
}
