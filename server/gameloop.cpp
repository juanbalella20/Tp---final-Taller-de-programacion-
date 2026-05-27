#include "gameloop.h"
#include "game_exceptions.h"

#include <iostream>
#include <cmath>

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
             //REPARACION NPC
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
                        item_infos.emplace_back(item->get_id(), item->getName(), item->getPrice());
                    }
                    GameMsg inv_msg(MSG_INVENTORY);
                    inv_msg.set_items(item_infos);
                    client_registry_monitor.notify_client(cmd.get_client_id(), inv_msg);
                    GameMsg gold_msg(MSG_GOLD);
                    gold_msg.set_gold(game_map.get_player_gold(cmd.get_player_name()));
                    client_registry_monitor.notify_client(cmd.get_client_id(), gold_msg);
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
                    std::string attacker_name = client_registry_monitor.get_name(cmd.get_client_id());
                    try {
                        auto result = game_map.attack(attacker_name, x, y);
                        if (result.entity_died) {
                            GameMsg map_msg(MSG_SEND_MAP);
                            map_msg.set_map(game_map.get_map());
                            client_registry_monitor.notify_clients(map_msg);
                        }
                    } catch (const NoEntityException& e) {
                        GameMsg msg(MSG_CHAT);
                        msg.set_chat_content(e.what());
                        client_registry_monitor.notify_client(cmd.get_client_id(), msg);
                    } catch (const AttackNotAllowedException& e) {
                        GameMsg msg(MSG_CHAT);
                        msg.set_chat_content(e.what());
                        client_registry_monitor.notify_client(cmd.get_client_id(), msg);
                    } catch (const NoWeaponEquippedException& e) {
                        GameMsg msg(MSG_CHAT);
                        msg.set_chat_content(e.what());
                        client_registry_monitor.notify_client(cmd.get_client_id(), msg);
                    } catch (const OutOfRangeException& e) {
                        GameMsg msg(MSG_CHAT);
                        msg.set_chat_content(e.what());
                        client_registry_monitor.notify_client(cmd.get_client_id(), msg);
                    }
                    break;
                }
                case MSG_EQUIP:{
                    std::string name =
                        client_registry_monitor.get_name(cmd.get_client_id());
                        std::string item_id = cmd.get_item_id();
                        game_map.player_equip_item(name, item_id);
                }
                case MSG_SELECT: {
                    std::string name =
                        client_registry_monitor.get_name(cmd.get_client_id());
                    uint16_t coor_x = cmd.get_coord_x();
                    uint16_t coor_y = cmd.get_coord_y();
                    std::cout << "[DEBUG: MSG_SELECT] col=" << coor_x
                              << " fila=" << coor_y << std::endl;
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
                case MSG_BUY:{
                    //recibis coordedas de donde tocaste 
                    //recibis el item-> cmd
                    // el player que toco-> cmd
                    //game_map.buy_item(player_name, item_id,coordenadas); -> lista de npc
                        //player.buy_item(item_id, npc);
                            //aca tenes info de ORO, NPC, ITEM_ID, 
                            //caso de npc con lista
                            //RESTAR EL ORO

                            //inventario.add(*item);
                    break; 
                }

                case MSG_TAKE: {
                    std::string name = client_registry_monitor.get_name(cmd.get_client_id());
                    auto item = game_map.pick_up_item(name);
                    GameMsg msg(MSG_TAKE);
                    if (item) {
                        game_map.give_item_to_player(name, std::move(item));
                        msg.set_chat_content("Recogiste un objeto.");
                        const Player& p = game_map.get_player(name);
                        std::vector<ItemInfo> item_infos;
                        for (Item* item : p.get_inventory().get_items()) {
                            item_infos.emplace_back(item->get_id(), item->getName(), item->getPrice());
                        }
                        GameMsg inv_msg(MSG_INVENTORY);
                        inv_msg.set_items(item_infos);
                        client_registry_monitor.notify_client(cmd.get_client_id(), inv_msg);
                    } else {
                        msg.set_chat_content("Sumaste oro.");
                        GameMsg gold_msg(MSG_GOLD);
                        gold_msg.set_gold(game_map.get_player_gold(name));
                        client_registry_monitor.notify_client(cmd.get_client_id(), gold_msg);
                    }
                    client_registry_monitor.notify_client(cmd.get_client_id(), msg);
                    
                    GameMsg map_msg(MSG_SEND_MAP);
                    map_msg.set_map(game_map.get_map());
                    client_registry_monitor.notify_clients(map_msg);
                    break;
                }
                //case MSG_RESURRECT: handle_resurrect(cmd); break;
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
/*
void GameLoop::handle_attack(const ClientCmd& cmd) {
    std::string attacker_name = client_registry_monitor.get_name(cmd.get_client_id());
    Player* attacker = game_map.get_player(attacker_name);
    if (!attacker || attacker->is_ghost()) return;
 
    // TODO: cuando NPCs estén implementados, manejar ENTITY_NPC
    if (cmd.get_target_type() != ENTITY_PLAYER) return;
 
    Player* target = game_map.get_player(cmd.get_target_name());
    if (!target || target->is_ghost()) return;
 
    // Fair play: ninguno puede ser newbie (nivel <= 12)
    if (attacker->get_level() <= 12 || target->get_level() <= 12) return;
 
    // Diferencia de nivel no puede superar 10
    if (std::abs(attacker->get_level() - target->get_level()) > 10) return;
 
    // Mismo clan no pueden atacarse.> //clan(jugador1/ jugadpr)
    if (attacker->get_clan_id() != -1 &&
        attacker->get_clan_id() == target->get_clan_id()) return;
 
    // Rango cuerpo a cuerpo: deben ser adyacentes
    // TODO: cuando Item esté definido, verificar si el arma es a distancia   //LOGICA DE ARMA
    int dx = std::abs(attacker->get_coord_x() - target->get_coord_x());
    int dy = std::abs(attacker->get_coord_y() - target->get_coord_y());
    if (dx + dy > 1) return;
 
    attacker->stop_meditation();
 
    int damage = attacker->damage_attack();
 
    // Esquiva: rand(0,1)^Agilidad < 0.001
    // TODO: usar agilidad real del target cuando este expuesta en Player
    bool evaded = false;
 
    if (evaded) {
        GameMsg evade_msg(MSG_ATTACK);
        evade_msg.set_player_name(attacker_name);
        client_registry_monitor.notify_client(cmd.get_client_id(), evade_msg);
        return;
    }
 
    target->recv_attack(damage);
 
    int exp = damage * std::max(target->get_level() - attacker->get_level() + 10, 0);
    attacker->add_experience(exp);
    attacker->check_level_up();
 
    GameMsg atk_msg(MSG_ATTACK);
    atk_msg.set_player_name(attacker_name);
    atk_msg.set_coord_x(damage);
    client_registry_monitor.notify_clients(atk_msg);
 
    if (target->get_lives() <= 0) {
        int exp_bonus = static_cast<int>(
            (rand() / (float)RAND_MAX) * 0.1f *
            std::max(target->get_level() - attacker->get_level() + 10, 0)
        );
        attacker->add_experience(exp_bonus);
        attacker->check_level_up();
 
        int oro_max = static_cast<int>(100 * std::pow(target->get_level(), 1.1));
        int exceso  = std::max(0, target->get_gold() - oro_max);
        if (exceso > 0) {
            target->give_gold(exceso);
            // TODO: game_map.add_gold_on_floor(target->get_coord_x(),
            //                                  target->get_coord_y(), exceso);
        }
 
        // TODO: tirar inventario al suelo cuando Item se mergee
 
        target->set_ghost();
 
        GameMsg dead_msg(MSG_ATTACK);
        dead_msg.set_player_name(target->get_name());
        client_registry_monitor.notify_clients(dead_msg);
 
        std::cout << "[INFO: MSG_ATTACK] " << target->get_name()
                  << " murio, matado por " << attacker_name << std::endl;
    }
 
    std::cout << "[INFO: MSG_ATTACK] " << attacker_name
              << " -> " << target->get_name()
              << " dmg=" << damage << std::endl;
}

void GameLoop::handle_meditate(const ClientCmd& cmd) {
    std::string name = client_registry_monitor.get_name(cmd.get_client_id());
    Player* player = game_map.get_player(name);
    if (!player) return;
 
    if (player->is_ghost()) return;
    if (!player->can_meditate()) return;
 
    player->change_meditation();
 
    GameMsg msg(MSG_MEDITATE);
    msg.set_player_name(name);
    client_registry_monitor.notify_client(cmd.get_client_id(), msg);
 
    std::cout << "[INFO: MSG_MEDITATE] " << name
              << (player->is_meditating() ? " empezo a meditar" : " dejo de meditar")
              << std::endl;
}

void GameLoop::handle_resurrect(const ClientCmd& cmd) {
    std::string name = client_registry_monitor.get_name(cmd.get_client_id());
    Player* player = game_map.get_player(name);
    if (!player) return;
 
    if (!player->is_ghost()) return;
 
    if (!cmd.get_target_name().empty() && cmd.get_target_type() == ENTITY_NPC) {
        // Caso B: tiene sacerdote seleccionado
        // TODO: verificar que el target es NpcPriest y que es adyacente
        position_coord spawn = game_map.get_spawn_position();
        player->revive();
        player->update_position(spawn.x, spawn.y);
 
        GameMsg msg(MSG_RESURRECT);
        msg.set_player_name(name);
        msg.set_coord_x(spawn.x);
        msg.set_coord_y(spawn.y);
        client_registry_monitor.notify_clients(msg);
 
        std::cout << "[INFO: MSG_RESURRECT] " << name
                  << " resucito en (" << spawn.x << "," << spawn.y << ")" << std::endl;
    } else {
        // Caso A: resurreccion remota con timer
        // TODO: calcular distancia al sacerdote mas cercano, inmovilizar al jugador y arrancar el timer.
        // Requiere tick pero no se si lo vamos a implementar
        std::cout << "[TODO: MSG_RESURRECT] resurreccion remota no implementada" << std::endl;
    }
*/

