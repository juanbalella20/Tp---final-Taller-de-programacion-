#include "gameloop.h"

#include <iostream>

#include "../common/gameMsg.h"

GameLoop::GameLoop(Queue<ClientCmd>& receiving_queue,
                   ClientRegistryMonitor& client_registry_monitor):
        receiving_queue(receiving_queue), client_registry_monitor(client_registry_monitor) {}


void GameLoop::load_world() {
    load_maps();
    game_map.load_players();
    //load_npcs();

}

void GameLoop::load_maps() {
    game_map.read_desert();
    game_map.read_city();
    game_map.read_forest();
    // TODO: cargar posiciones desde persistencia
    game_map.set_positions();
}

void GameLoop::run() {
    load_world();    
    while (should_keep_running()) {
        try {
            ClientCmd cmd = receiving_queue.pop();
            std::cout << "[DEBUG: gameloop] received cmd type="
                      << static_cast<int>(cmd.get_message_type())
                      << " name='" << cmd.get_player_name() << "'"
                      << "client_id" << cmd.get_client_id()
                      << std::endl;
            switch (cmd.get_message_type()) {
                case MSG_REGISTER: {
                    client_registry_monitor.assign_name(cmd.get_client_id(), cmd.get_player_name());
                    game_map.spawn_player(cmd.get_player_name());
                    GameMsg msg(MSG_SEND_MAP);
                    msg.set_map(game_map.get_map());
                    client_registry_monitor.notify_client(cmd.get_client_id(), msg);
                    std::cout << "[DEBUG: MSG_REGISTER] received cmd type="
                      << static_cast<int>(cmd.get_message_type())
                      << "client_id" << cmd.get_client_id()
                      << std::endl;
                    break;
                }
                case MSG_MOVE: {
                    std::string name =
                        client_registry_monitor.get_name(cmd.get_client_id());
                    auto result = game_map.try_move(cmd.get_direction(), name);
                    if (result.moved) {
                        GameMsg msg(MSG_MOVE, cmd.get_direction());
                        msg.set_player_name(result.player_name);
                        msg.set_coord_x(result.new_x);
                        msg.set_coord_y(result.new_y);
                        client_registry_monitor.notify_clients(msg);
                        std::cout << "[DBUG]: sended" << std::endl;
                    }
                    break;
                }
                case MSG_SELECT: {
                    std::string name =
                        client_registry_monitor.get_name(cmd.get_client_id());
                    uint16_t coor_x = cmd.get_coord_x();
                    uint16_t coor_y = cmd.get_coord_y();
                    std::cout << "[DEBUG: MSG_SELECT] received cmd type="
                      << static_cast<int>(cmd.get_message_type())
                      << "client_id" << cmd.get_client_id()
                      << "coor_x" << coor_x
                      << "coor_y" << coor_y
                      << std::endl;
                    //std::string sector = game_map.sector_of_position(coor_x, coor_y);
                    break;
                }
                case MSG_MEDITATE: {
                    /* lógica*/
                }
                /* etc con los demás*/
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

