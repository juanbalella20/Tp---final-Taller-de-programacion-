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
                    // TODO: crear el jugador
                    client_registry_monitor.assign_name(cmd.get_client_id(), cmd.get_player_name());
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
                case MSG_ATTACK: {
                    std::string attacker_name =
                        client_registry_monitor.get_name(cmd.get_client_id());
                    Player* attacker = game_map.get_player(attacker_name);
                    if (!attacker || attacker->is_ghost()) break;
                    if (cmd.get_target_type() != ENTITY_PLAYER) break;

                    // Solo soportamos ataque a jugadores por ahora
                    // TODO: cuando NPCs esten implementados, manejar ENTITY_NPC
                    if (cmd.get_target_type() != ENTITY_PLAYER) break;
 
                    Player* target = game_map.get_player(cmd.get_target_name());
                    if (!target || target->is_ghost()) break;
 
                    // Fair play: ninguno puede ser newbie (nivel <= 12)
                    if (attacker->get_level() <= 12 || target->get_level() <= 12) break;
 
                    // Diferencia de nivel no puede superar 10
                    if (std::abs(attacker->get_level() - target->get_level()) > 10) break;
 
                    // Mismo clan no pueden atacarse
                    if (attacker->get_clan_id() != -1 &&
                        attacker->get_clan_id() == target->get_clan_id()) break;
                    
                    // Validar rango (cuerpo a cuerpo: deben ser adyacentes)
                    // TODO: cuando Item este definido, verificar si el arma es a distancia
                    int dx = std::abs(attacker->get_coord_x() - target->get_coord_x());
                    int dy = std::abs(attacker->get_coord_y() - target->get_coord_y());
                    if (dx + dy > 1) break;

                    // Calcular daño
                    int damage = attacker->damage_attack();
 
                    // Esquiva: rand(0,1)^Agilidad < 0.001
                    // TODO: cuando PlayerRace exponga agilidad al gameloop,
                    // usar la agilidad real del target. Por ahora siempre aplica daño.
                    bool evaded = false;
 
                    if (evaded) {
                        GameMsg evade_msg(MSG_ATTACK);
                        evade_msg.set_player_name(attacker_name);
                        client_registry_monitor.notify_client(cmd.get_client_id(), evade_msg);
                        break;
                    }
 
                    target->recv_attack(damage);
 
                    // Experiencia al atacante
                    int exp = damage * std::max(target->get_level() - attacker->get_level() + 10, 0);
                    attacker->add_experience(exp);
                    attacker->check_level_up();

                    
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
