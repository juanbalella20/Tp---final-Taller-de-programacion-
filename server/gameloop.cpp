#include "gameloop.h"

#include "../common/gameMsg.h"

GameLoop::GameLoop(Queue<ClientCmd>& receiving_queue,
                   ClientRegistryMonitor& client_registry_monitor):
        receiving_queue(receiving_queue), client_registry_monitor(client_registry_monitor) {}

void GameLoop::run() {
    while (should_keep_running()) {
        try {
            ClientCmd cmd = receiving_queue.pop();
            switch (cmd.get_message_type()) {
                case MSG_REGISTER: {
                    client_registry_monitor.assign_name(cmd.get_client_id(), cmd.get_player_name());
                    GameMsg msg(MSG_SEND_MAP);
                    msg.set_map(game_map.get_map());
                    client_registry_monitor.notify_client(cmd.get_client_id(), msg);
                    break;
                }
                case MSG_MOVE: {
                    GameMsg msg(MSG_MOVE, cmd.get_direction());
                    client_registry_monitor.notify_clients(msg);
                    break;
                }
                default:
                    break;
            }
        } catch (const ClosedQueue&) {
            break;
        } catch (...) {
            // Ignore malformed/unexpected commands in minimal integration loop.
        }
    }
}
