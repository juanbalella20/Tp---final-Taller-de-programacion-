#include "gameloop.h"
#include "game_exceptions.h"

#include <iostream>
#include <cmath>
#include <chrono>
#include <thread>

#include "../common/commands/gameMsg.h"
#include "../common/info/item_info.h"
#include "Game/item/item.h"

GameLoop::GameLoop(Queue<ClientCmd>& receiving_queue,
                   ClientRegistryMonitor& client_registry_monitor):
        receiving_queue(receiving_queue), client_registry_monitor(client_registry_monitor) {
    register_handlers();
}

void GameLoop::register_handlers() {
    handlers[MSG_REGISTER]       = [this](const ClientCmd& cmd) { handle_register(cmd); };
    handlers[MSG_LIST]           = [this](const ClientCmd& cmd) { handle_list(cmd); };
    handlers[MSG_SELL]           = [this](const ClientCmd& cmd) { handle_sell(cmd); };
    handlers[MSG_BUY]            = [this](const ClientCmd& cmd) { handle_buy(cmd); };
    handlers[MSG_EQUIP]          = [this](const ClientCmd& cmd) { handle_equip(cmd); };
    handlers[MSG_SELECT]         = [this](const ClientCmd& cmd) { handle_select(cmd); };
    handlers[MSG_TAKE]           = [this](const ClientCmd& cmd) { handle_take(cmd); };
    handlers[MSG_MOVE]           = [this](const ClientCmd& cmd) { handle_move(cmd); };
    handlers[MSG_ATTACK]         = [this](const ClientCmd& cmd) { handle_attack(cmd); };
    handlers[MSG_MEDITATE]       = [this](const ClientCmd& cmd) { handle_meditate(cmd); };
    handlers[MSG_TELEPORT]       = [this](const ClientCmd& cmd) { handle_teleport(cmd); };
    handlers[MSG_PRIVATE]        = [this](const ClientCmd& cmd) { handle_private(cmd); };
    handlers[MSG_CHEAT_KILL]     = [this](const ClientCmd& cmd) { handle_cheat_kill(cmd); };
    handlers[MSG_CHEAT_INF_HP]   = [this](const ClientCmd& cmd) { handle_cheat_inf_hp(cmd); };
    handlers[MSG_CHEAT_INF_MANA] = [this](const ClientCmd& cmd) { handle_cheat_inf_mana(cmd); };
}


void GameLoop::load_world() {
    load_maps();

}

InitialState load_initial_state_hardcoded() {
    InitialState is;

    // NPCs de prueba (stats reales viven en make_npc_from_spawn).
    is.npcs.push_back({"goblin", 7, 5});
    is.npcs.push_back({"spider", 9, 5});

    // TODO: items hardcodeados en el piso
    return is;
}

void GameLoop::broadcast_npcs_snapshot() {
    // Cada cliente ve los NPCs de SU zona
    for (const auto& [client_id, name] : client_registry_monitor.get_active_clients()) {
        if (name.empty() || !game_map.player_exists(name)) continue;
        GameMsg msg(MSG_NPCS_SNAPSHOT);
        msg.set_npcs(game_map.build_npcs_snapshot(name));
        client_registry_monitor.notify_client(client_id, msg);
    }
}

void GameLoop::send_npcs_snapshot_to(uint32_t client_id) {
    std::string name = client_registry_monitor.get_name(client_id);
    GameMsg msg(MSG_NPCS_SNAPSHOT);
    msg.set_npcs(game_map.build_npcs_snapshot(name));
    client_registry_monitor.notify_client(client_id, msg);
}

void GameLoop::broadcast_items_snapshot() {
    // Cada cliente ve los items/oro del piso de SU zona
    for (const auto& [client_id, name] : client_registry_monitor.get_active_clients()) {
        if (name.empty() || !game_map.player_exists(name)) continue;
        GameMsg msg(MSG_ITEMS_SNAPSHOT);
        msg.set_items_on_floor(game_map.build_items_snapshot(name));
        client_registry_monitor.notify_client(client_id, msg);
    }
}

void GameLoop::send_items_snapshot_to(uint32_t client_id) {
    std::string name = client_registry_monitor.get_name(client_id);
    GameMsg msg(MSG_ITEMS_SNAPSHOT);
    msg.set_items_on_floor(game_map.build_items_snapshot(name));
    client_registry_monitor.notify_client(client_id, msg);
}

void GameLoop::load_maps() {
    // TODO:
    // funcion para persistencia
    // InitialState load_initial_state_from_file(path);

    // harcoded:
    InitialState hardocded_state = load_initial_state_hardcoded();
    std::map<Zone, std::string> zone_paths = {
        {ZONE_DESERT, "data/maps/desert/map.toml"},
        {ZONE_CITY,   "data/maps/city/map.toml"},
    };
    std::map<Zone, InitialState> initial_states = {
        {ZONE_DESERT, hardocded_state},
        {ZONE_CITY,   InitialState{}},  // city sin NPCs por ahora (testeo)
    };
    game_map.init_world(zone_paths, initial_states);
}


// registrarse
//                spawn del jugador con la klass y la race
//                enviar mapa al cliente
//                 mandamos inventario
//                 mandamos oro
//                 mandamos hp/xp/mana
// mandamos snapshot de npcs e items
// mandar snaphot de players


void GameLoop::send_player_snapshot_to_other_players(uint32_t client_id, const std::string& player_name) {
    // Avisa a todos los demás clientes que un nuevo jugador apareció.
    // Solo incluye al jugador recién registrado, no a todos.
    const Player& p = game_map.get_player(player_name);
    GameMsg msg(MSG_PLAYERS_SNAPSHOT);
    msg.set_player({player_name, 0, 0, p.get_coord_x(), p.get_coord_y()});
    std::cout << "[DEBUG: MSG_PLAYERS_SNAPSHOT] Notificando a otros jugadores sobre nuevo jugador " << msg.get_players().front().name << std::endl;
    client_registry_monitor.notify_clients_less_client(client_id, msg);
}

void GameLoop::process_cmd(const ClientCmd& cmd) {
    auto it = handlers.find(static_cast<uint8_t>(cmd.get_message_type()));
    if (it != handlers.end()) it->second(cmd);
}

void GameLoop::handle_register(const ClientCmd& cmd) {
    client_registry_monitor.assign_name(cmd.get_client_id(), cmd.get_player_name());
    game_map.spawn_player(cmd.get_player_name());
    GameMsg registerMsg(MSG_REGISTER);
    registerMsg.set_map(game_map.get_map(cmd.get_player_name()));

    std::cout << "[DEBUG: MSG_REGISTER] received cmd type="
              << static_cast<int>(cmd.get_message_type())
              << " client_id=" << cmd.get_client_id() << std::endl;

    const Player& p = game_map.get_player(cmd.get_player_name());
    std::vector<ItemInfo> item_infos;
    for (Item* item : p.get_inventory().get_items()) {
        item_infos.emplace_back(item->get_id(), item->getName(), item->getPrice());
    }
    registerMsg.set_items(item_infos);
    registerMsg.set_gold(game_map.get_player_gold(cmd.get_player_name()));
    registerMsg.set_hp(game_map.get_player_hp(cmd.get_player_name()));
    registerMsg.set_xp(game_map.get_player_xp(cmd.get_player_name()));
    registerMsg.set_mana(game_map.get_player_mana(cmd.get_player_name()));
    registerMsg.set_coord_x(p.get_coord_x());
    registerMsg.set_coord_y(p.get_coord_y());
    registerMsg.set_players(game_map.build_players_snapshot(cmd.get_player_name()));
    client_registry_monitor.notify_client(cmd.get_client_id(), registerMsg);

    send_npcs_snapshot_to(cmd.get_client_id());
    send_items_snapshot_to(cmd.get_client_id());
    send_player_snapshot_to_other_players(cmd.get_client_id(), cmd.get_player_name());//le quiero avisar a toods que se conecto el cliente 1

    // Envia la zona real donde spawneo el player
    GameMsg zoneMsg(MSG_ZONE_CHANGE);
    zoneMsg.set_zone(game_map.get_player_zone(cmd.get_player_name()));
    zoneMsg.set_coord_x(p.get_coord_x());
    zoneMsg.set_coord_y(p.get_coord_y());
    client_registry_monitor.notify_client(cmd.get_client_id(), zoneMsg);
}

void GameLoop::handle_list(const ClientCmd& cmd) {
    try {
        std::string name = client_registry_monitor.get_name(cmd.get_client_id());
        std::vector<ItemInfo> items = game_map.list_seller_items(name, 0, 0);
        GameMsg msg(MSG_CHAT);
        std::string lista = "Items disponibles: ";
        for (const auto& item : items) {
            lista += item.get_name() + " ($" + std::to_string(item.get_price()) + ") ";
        }
        msg.set_chat_content(lista);
        client_registry_monitor.notify_client(cmd.get_client_id(), msg);
    } catch (const std::runtime_error& e) {
        GameMsg msg(MSG_CHAT);
        msg.set_chat_content(e.what());
        client_registry_monitor.notify_client(cmd.get_client_id(), msg);
    }
}
 
void GameLoop::handle_sell(const ClientCmd& cmd) {
    std::string name = client_registry_monitor.get_name(cmd.get_client_id());
    std::string item_id = cmd.get_item_id();
    try {
        game_map.player_sell_item(name, 0, 0, item_id);
        const Player& p = game_map.get_player(name);
        std::vector<ItemInfo> item_infos;
        for (Item* item : p.get_inventory().get_items()) {
            item_infos.emplace_back(item->get_id(), item->getName(), item->getPrice());
        }
        GameMsg inv_msg(MSG_INVENTORY);
        inv_msg.set_items(item_infos);
        client_registry_monitor.notify_client(cmd.get_client_id(), inv_msg);
 
        GameMsg gold_msg(MSG_GOLD);
        gold_msg.set_gold(p.get_gold());
        client_registry_monitor.notify_client(cmd.get_client_id(), gold_msg);
    } catch (const std::runtime_error& e) {
        GameMsg msg(MSG_CHAT);
        msg.set_chat_content(e.what());
        client_registry_monitor.notify_client(cmd.get_client_id(), msg);
    }
}
 
void GameLoop::handle_buy(const ClientCmd& cmd) {
    std::string name = client_registry_monitor.get_name(cmd.get_client_id());
    std::string item_id = cmd.get_item_id();
    try {
        game_map.player_buy_item(name, 0, 0, item_id);
        const Player& p = game_map.get_player(name);
        std::vector<ItemInfo> item_infos;
        for (Item* item : p.get_inventory().get_items()) {
            item_infos.emplace_back(item->get_id(), item->getName(), item->getPrice());
        }
        GameMsg inv_msg(MSG_INVENTORY);
        inv_msg.set_items(item_infos);
        client_registry_monitor.notify_client(cmd.get_client_id(), inv_msg);
 
        GameMsg gold_msg(MSG_GOLD);
        gold_msg.set_gold(p.get_gold());
        client_registry_monitor.notify_client(cmd.get_client_id(), gold_msg);
    } catch (const std::runtime_error& e) {
        GameMsg msg(MSG_CHAT);
        msg.set_chat_content(e.what());
        client_registry_monitor.notify_client(cmd.get_client_id(), msg);
    }
}

void GameLoop::handle_equip(const ClientCmd& cmd) {
    std::string name = client_registry_monitor.get_name(cmd.get_client_id());
    game_map.player_equip_item(name, cmd.get_item_id());
}

void GameLoop::handle_select(const ClientCmd& cmd) {
    uint16_t coor_x = cmd.get_coord_x();
    uint16_t coor_y = cmd.get_coord_y();
    std::cout << "[DEBUG: MSG_SELECT] col=" << coor_x << " fila=" << coor_y << std::endl;
    selected_npc[cmd.get_client_id()] = {coor_x, coor_y};
}

void GameLoop::handle_take(const ClientCmd& cmd) {
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
    }
    client_registry_monitor.notify_client(cmd.get_client_id(), msg);
    broadcast_items_snapshot();
}

void GameLoop::handle_teleport(const ClientCmd& cmd) {
    std::string name = client_registry_monitor.get_name(cmd.get_client_id());
    auto tp_result = game_map.teleport_player(name);

    if (!tp_result.adyacente) {
        GameMsg msg(MSG_CHAT);
        msg.set_chat_content("No hay zona de teleportación cerca");
        client_registry_monitor.notify_client(cmd.get_client_id(), msg);
        return;
    }

    // Transición a la nueva zona
    GameMsg zoneMsg(MSG_ZONE_CHANGE);
    zoneMsg.set_zone(tp_result.dest_zone);
    zoneMsg.set_coord_x(tp_result.x);
    zoneMsg.set_coord_y(tp_result.y);
    client_registry_monitor.notify_client(cmd.get_client_id(), zoneMsg);

    // Nuevo terreno + actores/items de la nueva zona
    GameMsg mapMsg(MSG_SEND_MAP);
    mapMsg.set_map(game_map.get_map(name));
    client_registry_monitor.notify_client(cmd.get_client_id(), mapMsg);

    send_npcs_snapshot_to(cmd.get_client_id());
    send_items_snapshot_to(cmd.get_client_id());
}

void GameLoop::handle_move(const ClientCmd& cmd) {
    std::string name = client_registry_monitor.get_name(cmd.get_client_id());
    auto result = game_map.try_move(cmd.get_direction(), name);
    // if (result.moved) {
        if (game_map.pick_up_gold(name)) {
            std::cout << "oro" << std::endl;
            broadcast_items_snapshot();
            GameMsg msg_gold(MSG_GOLD);
            msg_gold.set_gold(game_map.get_player_gold(name));
            client_registry_monitor.notify_clients(msg_gold);
        }
        GameMsg msg(MSG_MOVE, cmd.get_direction());
        msg.set_player_name(result.player_name);
        msg.set_coord_x(result.new_x);
        msg.set_coord_y(result.new_y);
        client_registry_monitor.notify_clients(msg);
        std::cout << "[DBUG]: sended" << std::endl;
    // }
}

void GameLoop::handle_attack(const ClientCmd& cmd) {
    int x = cmd.get_coord_x();
    int y = cmd.get_coord_y();
    std::string attacker_name = client_registry_monitor.get_name(cmd.get_client_id());
    try {
        auto result = game_map.attack(attacker_name, x, y);
        if (result.entity_died) {
            broadcast_npcs_snapshot();
            broadcast_items_snapshot();
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
}

void GameLoop::handle_meditate(const ClientCmd& cmd) {
    std::string name = client_registry_monitor.get_name(cmd.get_client_id());
    GameMsg msg(MSG_MEDITATE);
    msg.set_chat_content("Estás meditando...");
    client_registry_monitor.notify_client(cmd.get_client_id(), msg);
}

void GameLoop::handle_private(const ClientCmd& cmd) {
    std::string sender = client_registry_monitor.get_name(cmd.get_client_id());
    GameMsg msg(MSG_PRIVATE);
    msg.set_player_name(sender);
    msg.set_chat_content(cmd.get_chat_content());
    //client_registry_monitor.notify_client_by_name(cmd.get_target_name(), msg);
}

void GameLoop::handle_cheat_kill(const ClientCmd& cmd) {
    std::string name = client_registry_monitor.get_name(cmd.get_client_id());
    // game_map.kill_player(name);
    GameMsg msg(MSG_CHEAT_KILL);
    msg.set_chat_content("Moriste instantáneamente.");
    client_registry_monitor.notify_client(cmd.get_client_id(), msg);
}

void GameLoop::handle_cheat_inf_hp(const ClientCmd& cmd) {
    std::string name = client_registry_monitor.get_name(cmd.get_client_id());
    // game_map.set_infinite_hp(name);
    GameMsg msg(MSG_CHAT);
    msg.set_chat_content("Vida infinita activada.");
    client_registry_monitor.notify_client(cmd.get_client_id(), msg);
}

void GameLoop::handle_cheat_inf_mana(const ClientCmd& cmd) {
    std::string name = client_registry_monitor.get_name(cmd.get_client_id());
    // game_map.set_infinite_mana(name);
    GameMsg msg(MSG_CHAT);
    msg.set_chat_content("Mana infinito activado.");
    client_registry_monitor.notify_client(cmd.get_client_id(), msg);
}

void GameLoop::update_npcs_in_map(){
    if (game_map.update_npcs()) {
        broadcast_npcs_snapshot();
    }
}

// Relacion ticks/tiempo:
//   tick_rate = 50ms  =>  20 ticks por segundo
//   Para un respawn de 5s: 5000ms / 50ms = 100 ticks
//   Para un respawn de 2s: 2000ms / 50ms = 40 ticks
//
// Como funciona sleep_until(next_tick):
//   next_tick es un punto fijo en el tiempo (no una duracion).
//   Al inicio de cada iteracion se adelanta 50ms: next_tick += 50ms.
//   Al final, sleep_until duerme lo que resta hasta ese punto.
//   Si procesar comandos tardo 3ms  -> duerme 47ms  (total: 50ms)
//   Si procesar comandos tardo 49ms -> duerme  1ms  (total: 50ms)
//   Si tardo mas de 50ms            -> no duerme, arranca el siguiente tick de inmediato
//   A diferencia de sleep_for(50ms), no acumula drift entre ticks.
void GameLoop::run() {
    load_world();

    const auto tick_rate = std::chrono::milliseconds(50); // 20 ticks/s
    auto next_tick = std::chrono::steady_clock::now();

    while (should_keep_running()) {
        next_tick += tick_rate;

        try {
            ClientCmd cmd;
            while (receiving_queue.try_pop(cmd)) {
                process_cmd(cmd);
            }
            update_npcs_in_map();
        }
        catch (... ) {
            //  
        }
        std::this_thread::sleep_until(next_tick);
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

