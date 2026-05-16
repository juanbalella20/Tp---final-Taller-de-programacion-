#include "GUIThread.h"

GUIThread::GUIThread(Queue<ClientCmd>& outgoing, Queue<GameMsg>& receiving)
    : outgoing(outgoing), receiving(receiving) {}

void GUIThread::run() {
    try {
        while (should_keep_running()) {
            // TODO: procesar eventos de SDL / input del usuario
            // Ejemplo: ClientCmd cmd = ...; outgoing.push(cmd);

            // TODO: consumir GameMsg de incoming y redibujar
            // Ejemplo: GameMsg msg; if (incoming.try_pop(msg)) { render(msg); }
        }
    } catch (const ClosedQueue&) {}
}
