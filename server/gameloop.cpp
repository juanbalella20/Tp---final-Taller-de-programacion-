#include "gameloop.h"

#include <iostream>

#include "../common/gameMsg.h"
#include "../common/item_info.h"
#include "item.h"

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
                    const Player& p = game_map.get_player(cmd.get_player_name());
                    std::vector<ItemInfo> item_infos;
                    for (Item* item : p.get_inventory().get_items()) {
                        item_infos.emplace_back(item->getId(), item->getName(), item->getPrice());
                    }
                    GameMsg inv_msg(MSG_INVENTORY);
                    inv_msg.set_items(item_infos);
                    client_registry_monitor.notify_client(cmd.get_client_id(), inv_msg);
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
                case MSG_ATTACK: {
                    int x = cmd.get_coord_x();
                    int y = cmd.get_coord_y();
                    auto result = game_map.attack_npc(x, y);
                    std::cout << "[DEBUG: MSG_ATTACK] x=" << x << " y=" << y
                              << " hit=" << result.hit
                              << " died=" << result.npc_died << std::endl;
                    if (result.hit) {
                        // Manda el mapa actualizado (sin el NPC) a todos los clientes
                        GameMsg map_msg(MSG_SEND_MAP);
                        map_msg.set_map(game_map.get_map());
                        client_registry_monitor.notify_clients(map_msg);
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
                    std::string name = client_registry_monitor.get_name(cmd.get_client_id());
                    // meditar al jugador : game_map.medidate_player(name) ?
                    GameMsg msg(MSG_MEDITATE);
                    msg.set_chat_content("Estás meditando...");
                    client_registry_monitor.notify_client(cmd.get_client_id(), msg);
                    break;
                }
                /* etc con los demás*/
                case MSG_CHEAT_KILL: {
                    std::string name = client_registry_monitor.get_name(cmd.get_client_id());
                    // game_map.kill_player(name);
                    GameMsg msg(MSG_CHAT);
                    msg.set_chat_content("Moriste instantáneamente.");
                    client_registry_monitor.notify_client(cmd.get_client_id(), msg);
                    break;
                }
                case MSG_CHEAT_INF_HP: {
                    std::string name = client_registry_monitor.get_name(cmd.get_client_id());
                    // game_map.set_infinite_hp(name);
                    GameMsg msg(MSG_CHAT);
                    msg.set_chat_content("Vida infinita activada.");
                    client_registry_monitor.notify_client(cmd.get_client_id(), msg);
                    break;
                }
                case MSG_CHEAT_INF_MANA: {
                    std::string name = client_registry_monitor.get_name(cmd.get_client_id());
                    // game_map.set_infinite_mana(name);
                    GameMsg msg(MSG_CHAT);
                    msg.set_chat_content("Mana infinito activado.");
                    client_registry_monitor.notify_client(cmd.get_client_id(), msg);
                    break;
                }
                case MSG_PRIVATE: {
                    std::string sender = client_registry_monitor.get_name(cmd.get_client_id());
                    std::string target = cmd.get_target_name();
                    GameMsg msg(MSG_PRIVATE);
                    msg.set_player_name(sender);
                    msg.set_chat_content(cmd.get_chat_content());
                    //client_registry_monitor.notify_client_by_name(target, msg);
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

/*
Avisar sobre reaparición de NPCs:

std::vector<std::string> npcs_respawned = game_map.update_respawns()
for (const auto& npc : npcs_respawned) {
    GameMsg msg(MSG_CHAT)
    msg.set_chat_content(npc + " ha reaparecido.")
    client_registry_monitor.notify_clients(msg)
}

(y algo así para la reaparición de los players...)
*/
