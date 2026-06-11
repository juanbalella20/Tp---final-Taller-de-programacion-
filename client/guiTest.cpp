#include "clientGUI.h"
#include "../common/queue.h"
#include "../common/clientCmd.h"

int main() {
    Queue<ClientCmd> outgoing;
    Queue<GameMsg> receiving;
    ClientGUI gui(outgoing, receiving);
    gui.run();
    return 0;
}
