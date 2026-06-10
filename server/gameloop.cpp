#include "gameloop.h"
#include "game_exceptions.h"

#include <iostream>
#include <cmath>
#include <chrono>
#include <thread>
#include <cstring>

#include "../common/commands/gameMsg.h"
#include "../common/info/item_info.h"
#include "Game/item/item.h"
#include "../common/constants/game_config.h"

GameLoop::GameLoop(Queue<ClientCmd>& receiving_queue,
                   ClientRegistryMonitor& client_registry_monitor):
        receiving_queue(receiving_queue), client_registry_monitor(client_registry_monitor),
        persistence(PERSIST_DATA_DIR), clan_persistence(PERSIST_DATA_DIR), auth(persistence) {
    register_handlers();
    load_persisted_clans();
}

void GameLoop::register_handlers() {
    handlers[MSG_REGISTER]       = [this](const ClientCmd& cmd) { handle_register(cmd); };
    handlers[MSG_LOGIN]          = [this](const ClientCmd& cmd) { handle_login(cmd); };
    handlers[MSG_LOGOUT]         = [this](const ClientCmd& cmd) { handle_logout(cmd); };
    handlers[MSG_LIST]           = [this](const ClientCmd& cmd) { handle_list(cmd); };
    handlers[MSG_SELL]           = [this](const ClientCmd& cmd) { handle_sell(cmd); };
    handlers[MSG_BUY]            = [this](const ClientCmd& cmd) { handle_buy(cmd); };
    handlers[MSG_DEPOSIT]        = [this](const ClientCmd& cmd) { handle_deposit(cmd); };
    handlers[MSG_DEP_GOLD]       = [this](const ClientCmd& cmd) { handle_deposit_gold(cmd); };
    handlers[MSG_RETIRE]         = [this](const ClientCmd& cmd) { handle_retire_item(cmd); };
    handlers[MSG_RET_GOLD]       = [this](const ClientCmd& cmd) { handle_retire_gold(cmd); };
    handlers[MSG_EQUIP]          = [this](const ClientCmd& cmd) { handle_equip(cmd); };
    handlers[MSG_SELECT]         = [this](const ClientCmd& cmd) { handle_select(cmd); };
    handlers[MSG_TAKE]           = [this](const ClientCmd& cmd) { handle_take(cmd); };
    handlers[MSG_MOVE]           = [this](const ClientCmd& cmd) { handle_move(cmd); };
    handlers[MSG_ATTACK]         = [this](const ClientCmd& cmd) { handle_attack(cmd); };
    handlers[MSG_MEDITATE]       = [this](const ClientCmd& cmd) { handle_meditate(cmd); };
    handlers[MSG_SELF_CAST]      = [this](const ClientCmd& cmd) { handle_self_cast(cmd); };
    handlers[MSG_USE_ITEM]       = [this](const ClientCmd& cmd) { handle_use_item(cmd); };
    handlers[MSG_TELEPORT]       = [this](const ClientCmd& cmd) { handle_teleport(cmd); };
    handlers[MSG_PRIVATE]        = [this](const ClientCmd& cmd) { handle_private(cmd); };
    handlers[MSG_CHEAT_KILL]     = [this](const ClientCmd& cmd) { handle_cheat_kill(cmd); };
    handlers[MSG_CHEAT_INF_HP]   = [this](const ClientCmd& cmd) { handle_cheat_inf_hp(cmd); };
    handlers[MSG_CHEAT_INF_MANA] = [this](const ClientCmd& cmd) { handle_cheat_inf_mana(cmd); };
    handlers[MSG_CHEAT_MANA]     = [this](const ClientCmd& cmd) { handle_cheat_mana(cmd); };
    handlers[MSG_CHEAT_RESPAWN]  = [this](const ClientCmd& cmd) { handle_cheat_revive(cmd); };
    handlers[MSG_FOUND_CLAN]     = [this](const ClientCmd& cmd) { handle_clan_foundation(cmd); };
    handlers[MSG_JOIN_CLAN]     = [this](const ClientCmd& cmd) { handle_clan_joining(cmd); };
    handlers[MSG_REV_CLAN]     = [this](const ClientCmd& cmd) { handle_clan_reviewing(cmd); };
    handlers[MSG_CLAN_ACEP]     = [this](const ClientCmd& cmd) { handle_clan_accepting(cmd); };
    handlers[MSG_CLAN_RECH]     = [this](const ClientCmd& cmd) { handle_clan_rejecting(cmd); };
    handlers[MSG_LEFT_CLAN]     = [this](const ClientCmd& cmd) { handle_clan_leaving(cmd); };
    handlers[MSG_CLAN_KICK]     = [this](const ClientCmd& cmd) { handle_clan_kick(cmd); };
    handlers[MSG_CLAN_BAN]     = [this](const ClientCmd& cmd) { handle_clan_ban(cmd); };
}


void GameLoop::load_world() {
    load_maps();

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
    // Cada zona se inicializa con su receta de poblado, leida ENTERA de
    // config.toml ([zones.<nombre>]): path del .bin, tipos de NPC permitidos,
    // cantidades y NPCs amigos. El cliente lee la misma fuente, asi servidor y
    // cliente nunca cargan mapas distintos.
    const auto& cfg = GameConfig::instance();
    std::map<Zone, ZoneSpawnConfig> zone_configs;
    for (const auto& [zone, path] : cfg.zone_map_paths) {
        ZoneSpawnConfig zc;
        zc.terrain_path   = path;
        zc.npc_types      = cfg.zone_allowed_npcs.at(zone);
        zc.item_types     = cfg.zone_allowed_items.at(zone);
        zc.num_npc        = cfg.zone_num_npc.at(zone);
        zc.num_items      = cfg.zone_num_items.at(zone);
        zc.num_priests    = cfg.zone_num_priests.at(zone);
        zc.num_sellers    = cfg.zone_num_sellers.at(zone);
        zc.num_bankers    = cfg.zone_num_bankers.at(zone);
        zone_configs.emplace(zone, std::move(zc));
    }
    game_map.init_world(zone_configs);
}


// registrarse
//                spawn del jugador con la klass y la race
//                enviar mapa al cliente
//                 mandamos inventario
//                 mandamos oro
//                 mandamos hp/xp/mana
// mandamos snapshot de npcs e items
// mandar snaphot de players


void GameLoop::notify_zone(Zone zone, const GameMsg& msg, const std::string& except_name) {
    // Visibilidad por zona: solo ve el evento quien comparte la zona 'zone'.
    for (const auto& [client_id, name] : client_registry_monitor.get_active_clients()) {
        if (name.empty() || name == except_name) continue;
        if (!game_map.player_exists(name)) continue;
        if (game_map.get_player_zone(name) != zone) continue;
        client_registry_monitor.notify_client(client_id, msg);
    }
}

void GameLoop::send_players_snapshot_to(uint32_t client_id, const std::string& player_name) {
    GameMsg msg(MSG_PLAYERS_SNAPSHOT);
    msg.set_players(game_map.build_players_snapshot(player_name));
    client_registry_monitor.notify_client(client_id, msg);
}

void GameLoop::send_player_snapshot_to_other_players(uint32_t client_id, const std::string& player_name, const std::string& player_race) {
    // Avisa a los demás clientes DE LA MISMA ZONA que un nuevo jugador apareció.
    // Solo incluye al jugador recién registrado, no a todos.
    (void)client_id;  // ahora se filtra por nombre/zona, no por id
    const Player& p = game_map.get_player(player_name);
    GameMsg msg(MSG_PLAYERS_SNAPSHOT);
    PlayerInfo pi{player_name, player_race, 0, p.get_coord_x(), p.get_coord_y()};
    pi.ghost = p.is_ghost();
    msg.set_player(pi);
    std::cout << "[DEBUG: MSG_PLAYERS_SNAPSHOT] Notificando a la zona sobre nuevo jugador " << msg.get_players().front().name << std::endl;
    notify_zone(game_map.get_player_zone(player_name), msg, player_name);
}

void GameLoop::process_cmd(const ClientCmd& cmd) {
    auto it = handlers.find(static_cast<uint8_t>(cmd.get_message_type()));
    if (it != handlers.end()) it->second(cmd);
}

void GameLoop::send_auth_error(uint32_t client_id, const std::string& reason) {
    GameMsg msg(MSG_AUTH_ERROR);
    msg.set_chat_content(reason);
    client_registry_monitor.notify_client(client_id, msg);
}

void GameLoop::send_confirm_session(uint32_t client_id, const std::string& name,
                                    const std::string& race, const std::string& klass) {
    GameMsg msg(MSG_CONFIRM_SESSION);
    msg.set_player_name(name);
    msg.set_race(race);
    msg.set_class(klass);
    client_registry_monitor.notify_client(client_id, msg);
}

void GameLoop::send_world_snapshot_to(uint32_t client_id, const std::string& name,
                                      const std::string& race) {
    GameMsg registerMsg(MSG_REGISTER);
    registerMsg.set_map(game_map.get_map(name));

    const Player& p = game_map.get_player(name);
    std::vector<ItemInfo> item_infos;
    for (Item* item : p.get_all_items()) {
        item_infos.emplace_back(item->get_id(), item->getName(), item->getPrice(),
                                static_cast<uint8_t>(item->get_type()), item->get_uid());
    }
    registerMsg.set_items(item_infos);
    registerMsg.set_gold(game_map.get_player_gold(name));
    registerMsg.set_hp(game_map.get_player_hp(name));
    registerMsg.set_max_hp(game_map.get_player_max_hp(name));
    registerMsg.set_xp(game_map.get_player_xp(name));
    registerMsg.set_mana(game_map.get_player_mana(name));
    registerMsg.set_max_mana(game_map.get_player_max_mana(name));
    std::cout << "[DEBUG] max xp: " << p.max_xp() << std::endl;
    registerMsg.set_max_xp(game_map.player_max_xp(name));
    registerMsg.set_coord_x(p.get_coord_x());
    registerMsg.set_coord_y(p.get_coord_y());
    registerMsg.set_level(p.get_level());
    registerMsg.set_ghost(p.is_ghost());
    registerMsg.set_players(game_map.build_players_snapshot(name));
    client_registry_monitor.notify_client(client_id, registerMsg);
    send_npcs_snapshot_to(client_id);
    send_items_snapshot_to(client_id);
    // Avisar a los demas que este jugador se conecto.
    send_player_snapshot_to_other_players(client_id, name, race);

    // Envia la zona real donde esta el player.
    GameMsg zoneMsg(MSG_ZONE_CHANGE);
    zoneMsg.set_zone(game_map.get_player_zone(name));
    zoneMsg.set_coord_x(p.get_coord_x());
    zoneMsg.set_coord_y(p.get_coord_y());
    client_registry_monitor.notify_client(client_id, zoneMsg);
    // El cliente limpia other_players al recibir MSG_ZONE_CHANGE, así que el
    // snapshot que viajó en MSG_REGISTER queda descartado: reenviarlo después.
    send_players_snapshot_to(client_id, name);
}

void GameLoop::handle_register(const ClientCmd& cmd) {
    const std::string& name = cmd.get_player_name();
    uint32_t client_id = cmd.get_client_id();

    AuthResult res = auth.try_register(name, cmd.get_password());
    if (res.status == AuthStatus::NAME_TAKEN) {
        std::cout << "[REGISTER] nombre ya existe: " << name << std::endl;
        send_auth_error(client_id, "El nombre ya existe");
        return;
    }

    // Nombre libre: crear el personaje y persistirlo de inmediato (red de
    // seguridad ante una desconexion antes del primer guardado periodico).
    client_registry_monitor.assign_name(client_id, name);
    game_map.spawn_player(name, cmd.get_race(), cmd.get_class());
    persistence.save(name, player_serializer.to_record(
            game_map.get_player(name), game_map.get_player_zone(name), cmd.get_password()));
    std::cout << "[REGISTER] nuevo personaje creado y persistido: " << name << std::endl;

    // Confirmacion de auth EXITOSO antes del world snapshot: el cliente lee
    // exactamente un mensaje de auth (MSG_AUTH_ERROR o MSG_CONFIRM_SESSION).
    send_confirm_session(client_id, name, cmd.get_race(), cmd.get_class());
    send_world_snapshot_to(client_id, name, cmd.get_race());
}

void GameLoop::handle_login(const ClientCmd& cmd) {
    const std::string& name = cmd.get_player_name();
    uint32_t client_id = cmd.get_client_id();

    AuthResult res = auth.try_login(name, cmd.get_password());
    if (res.status == AuthStatus::BAD_CREDENTIALS) {
        std::cout << "[LOGIN] credenciales invalidas: " << name << std::endl;
        send_auth_error(client_id, "Usuario o contraseña incorrectos");
        return;
    }

    client_registry_monitor.assign_name(client_id, name);

    // Re-login defensivo: si la copia vieja sigue viva en memoria, se reutiliza;
    // si no, se carga del record que devolvio el auth.
    if (!game_map.player_exists(name)) {
        Player player = player_serializer.from_record(res.record);
        game_map.add_persisted_player(std::move(player), static_cast<Zone>(res.record.zone));
        std::cout << "[LOGIN] personaje cargado de disco: " << name << std::endl;
    } else {
        std::cout << "[LOGIN] personaje reutilizado de memoria: " << name << std::endl;
    }

    const Player& player = game_map.get_player(name);
    const std::string& race = player.get_race_name();
    // La clase vive en el server (enum del dominio); la traducimos al string del
    // protocolo para que el cliente la reciba en el confirm.
    std::string klass = CLASS_MAP_INV.at(static_cast<uint8_t>(player.get_class_id()));

    // Confirmacion de auth EXITOSO antes del world snapshot (ver handle_register).
    send_confirm_session(client_id, name, race, klass);
    send_world_snapshot_to(client_id, name, race);
}

void GameLoop::handle_logout(const ClientCmd& cmd) {
    // El nombre viaja en el comando sintético (lo puso el ClientHandler antes de
    // sacar al cliente del registro): no dependemos del registry, que ya pudo
    // haberse limpiado para este client_id.
    const std::string& name = cmd.get_player_name();
    if (name.empty() || !game_map.player_exists(name)) {
        selected_npc.erase(cmd.get_client_id());
        return;
    }

    // Persistir antes de borrarlo de memoria: red de seguridad ante una
    // desconexión entre guardados periódicos. La password ya está en disco;
    // la recuperamos para no pisarla (el Player no la conoce).
    PlayerRecord prev;
    std::string password;
    if (persistence.load(name, prev)) {
        password = std::string(prev.password, ::strnlen(prev.password, PERSIST_PASSWORD_MAX));
    }
    persistence.save(name, player_serializer.to_record(
            game_map.get_player(name), game_map.get_player_zone(name), password));

    // Capturar la zona ANTES de sacarlo: los que quedan en esa zona lo borran.
    Zone zone = game_map.get_player_zone(name);
    game_map.remove_player(name);
    selected_npc.erase(cmd.get_client_id());

    GameMsg leftMsg(MSG_PLAYER_LEFT);
    leftMsg.set_player_name(name);
    notify_zone(zone, leftMsg, name);
    std::cout << "[LOGOUT] " << name << " se desconectó y salió del mundo." << std::endl;
}

void GameLoop::handle_list(const ClientCmd& cmd) {
    try {
        std::string name = client_registry_monitor.get_name(cmd.get_client_id());
        std::string type = game_map.get_adjacent_npc_type(name);
        std::string lista = "";
        if (type == "seller") {
            std::vector<ItemInfo> items = game_map.list_seller_items(name, 0, 0);
            lista = "Items disponibles: ";
            for (const auto& item : items) {
                lista += item.get_name() + " ($" + std::to_string(item.get_price()) + ") ";
            }
        } else if (type == "banker") {
            std::vector<ItemInfo> items = game_map.list_banker_items(name);
            int gold = game_map.get_banker_gold(name);
            lista = "Banco - Oro: " + std::to_string(gold) + " Items: ";
            for (const auto& item : items) {
                lista += item.get_name() + " ";
            }
        } else {
            lista = "No hay un NPC adyacente.";
        }
        GameMsg msg(MSG_CHAT);
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
        for (Item* item : p.get_all_items()) {
            item_infos.emplace_back(item->get_id(), item->getName(), item->getPrice(), static_cast<uint8_t>(item->get_type()), item->get_uid());
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
        for (Item* item : p.get_all_items()) {
            item_infos.emplace_back(item->get_id(), item->getName(), item->getPrice(), static_cast<uint8_t>(item->get_type()), item->get_uid());
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

void GameLoop::handle_deposit(const ClientCmd& cmd) {
    std::string name = client_registry_monitor.get_name(cmd.get_client_id());
    std::string item_id = cmd.get_item_id();
    try {
        game_map.player_deposit_item(name, item_id);
        const Player& p = game_map.get_player(name);
        std::vector<ItemInfo> item_infos;
        for (Item* item : p.get_all_items()) {
            item_infos.emplace_back(item->get_id(), item->getName(), item->getPrice(), static_cast<uint8_t>(item->get_type()), item->get_uid());
        }
        GameMsg inv_msg(MSG_INVENTORY);
        inv_msg.set_items(item_infos);
        client_registry_monitor.notify_client(cmd.get_client_id(), inv_msg);
        GameMsg chat_msg(MSG_CHAT);
        chat_msg.set_chat_content("Item depositado en el banco.");
        client_registry_monitor.notify_client(cmd.get_client_id(), chat_msg);
    } catch (const std::runtime_error& e) {
        GameMsg msg(MSG_CHAT);
        msg.set_chat_content(e.what());
        client_registry_monitor.notify_client(cmd.get_client_id(), msg);
    }
}

void GameLoop::handle_deposit_gold(const ClientCmd& cmd) {
    std::string name = client_registry_monitor.get_name(cmd.get_client_id());
    int amount = cmd.get_gold();
    try {
        game_map.player_deposit_gold(name, amount);
        GameMsg gold_msg(MSG_GOLD);
        gold_msg.set_gold(game_map.get_player(name).get_gold());
        client_registry_monitor.notify_client(cmd.get_client_id(), gold_msg);
        GameMsg chat_msg(MSG_CHAT);
        chat_msg.set_chat_content("Oro depositado en el banco.");
        client_registry_monitor.notify_client(cmd.get_client_id(), chat_msg);
    } catch (const std::runtime_error& e) {
        GameMsg msg(MSG_CHAT);
        msg.set_chat_content(e.what());
        client_registry_monitor.notify_client(cmd.get_client_id(), msg);
    }
}

void GameLoop::handle_retire_item(const ClientCmd& cmd) {
    std::string name = client_registry_monitor.get_name(cmd.get_client_id());
    std::string item_id = cmd.get_item_id();
    try {
        game_map.player_retire_item(name, item_id);
        const Player& p = game_map.get_player(name);
        std::vector<ItemInfo> item_infos;
        for (Item* item : p.get_inventory().get_items()) {
            item_infos.emplace_back(item->get_id(), item->getName(), item->getPrice());
        }
        GameMsg inv_msg(MSG_INVENTORY);
        inv_msg.set_items(item_infos);
        client_registry_monitor.notify_client(cmd.get_client_id(), inv_msg);
        GameMsg chat_msg(MSG_CHAT);
        chat_msg.set_chat_content("Item retirado del banco.");
        client_registry_monitor.notify_client(cmd.get_client_id(), chat_msg);
    } catch (const std::runtime_error& e) {
        GameMsg msg(MSG_CHAT);
        msg.set_chat_content(e.what());
        client_registry_monitor.notify_client(cmd.get_client_id(), msg);
    }
}

void GameLoop::handle_retire_gold(const ClientCmd& cmd) {
    std::string name = client_registry_monitor.get_name(cmd.get_client_id());
    int amount = cmd.get_gold();
    try {
        game_map.player_retire_gold(name, amount);
        GameMsg gold_msg(MSG_GOLD);
        gold_msg.set_gold(game_map.get_player(name).get_gold());
        client_registry_monitor.notify_client(cmd.get_client_id(), gold_msg);
        GameMsg chat_msg(MSG_CHAT);
        chat_msg.set_chat_content("Oro retirado del banco.");
        client_registry_monitor.notify_client(cmd.get_client_id(), chat_msg);
    } catch (const std::runtime_error& e) {
        GameMsg msg(MSG_CHAT);
        msg.set_chat_content(e.what());
        client_registry_monitor.notify_client(cmd.get_client_id(), msg);
    }
}

void GameLoop::handle_equip(const ClientCmd& cmd) {
    std::string name = client_registry_monitor.get_name(cmd.get_client_id());
    // El cliente manda el uid de INSTANCIA del item a equipar (en texto decimal).
    uint64_t item_uid = 0;
    try {
        item_uid = std::stoull(cmd.get_item_id());
    } catch (const std::exception&) {
        return;  // uid mal formado: ignorar el comando
    }
    game_map.player_equip_item(name, item_uid);
    broadcast_player_equipment(name);
}

void GameLoop::broadcast_player_equipment(const std::string& player_name) {
    const Player& player = game_map.get_player(player_name);
    // equipped_ids: type_ids (para el sprite del personaje y detectar báculos).
    // equipped_uids: uids de instancia (para que el HUD resalte el slot exacto).
    // Ambas listas viajan como strings; van en paralelo (mismo orden de items).
    std::vector<uint64_t> uids = player.get_equipped_uids();
    std::vector<std::string> uid_strs;
    uid_strs.reserve(uids.size());
    for (uint64_t u : uids) uid_strs.push_back(std::to_string(u));

    GameMsg msg_equip(MSG_UPDATE_EQUIP);
    msg_equip.set_player_name(player_name);
    msg_equip.set_equipped(player.has_weapon_equipped());
    msg_equip.set_equipped_ids(player.get_equipped_type_ids());
    msg_equip.set_equipped_uids(uid_strs);

    client_registry_monitor.notify_clients(msg_equip);
}

void GameLoop::handle_select(const ClientCmd& cmd) {
    uint16_t coor_x = cmd.get_coord_x();
    uint16_t coor_y = cmd.get_coord_y();
    std::cout << "[DEBUG: MSG_SELECT] col=" << coor_x << " fila=" << coor_y << std::endl;
    selected_npc[cmd.get_client_id()] = {coor_x, coor_y};
}

void GameLoop::handle_take(const ClientCmd& cmd) {
    std::string name = client_registry_monitor.get_name(cmd.get_client_id());
    GameMsg msg(MSG_TAKE);
    // Tope de inventario (config.toml: player.max_inventory_slots). Si ya esta
    // lleno NO levantamos el item del piso, para no perderlo: pick_up_item lo
    // saca del suelo y add_item lo descartaria silenciosamente.
    if (game_map.get_player(name).get_inventory().is_full()) {
        msg.set_chat_content("Inventario lleno.");
        client_registry_monitor.notify_client(cmd.get_client_id(), msg);
        return;
    }
    auto item = game_map.pick_up_item(name);
    if (item) {
        game_map.give_item_to_player(name, std::move(item));
        msg.set_chat_content("Recogiste un objeto.");
        const Player& p = game_map.get_player(name);
        std::vector<ItemInfo> item_infos;
        for (Item* item : p.get_all_items()) {
            item_infos.emplace_back(item->get_id(), item->getName(), item->getPrice(), static_cast<uint8_t>(item->get_type()), item->get_uid());
        }
        GameMsg inv_msg(MSG_INVENTORY);
        inv_msg.set_items(item_infos);
        client_registry_monitor.notify_client(cmd.get_client_id(), inv_msg);
    }
    client_registry_monitor.notify_client(cmd.get_client_id(), msg);
    broadcast_items_snapshot();
}

void GameLoop::handle_teleport(const ClientCmd& cmd) {
    // CHEAT /tp <zona>: fuerza el cambio a la zona pedida (sin adyacencia).
    std::string name = client_registry_monitor.get_name(cmd.get_client_id());
    Zone dest = static_cast<Zone>(cmd.get_zone());
    auto tp_result = game_map.force_zone_change(name, dest);

    if (!tp_result.on_tile) {
        GameMsg msg(MSG_CHAT);
        msg.set_chat_content("Zona no disponible");
        client_registry_monitor.notify_client(cmd.get_client_id(), msg);
        return;
    }
    send_zone_transition(cmd.get_client_id(), name, tp_result);
}

void GameLoop::send_zone_transition(uint32_t client_id, const std::string& name,
                                    const TeleportResult& r) {
    // Cambio de zona fallido (zona no cargada o sin celda): no se envia nada.
    if (!r.on_tile) return;

    // Los que quedan en la zona de origen dejan de ver a este player.
    GameMsg leftMsg(MSG_PLAYER_LEFT);
    leftMsg.set_player_name(name);
    notify_zone(r.src_zone, leftMsg, name);

    // Transición a la nueva zona
    GameMsg zoneMsg(MSG_ZONE_CHANGE);
    zoneMsg.set_zone(r.dest_zone);
    zoneMsg.set_coord_x(r.x);
    zoneMsg.set_coord_y(r.y);
    client_registry_monitor.notify_client(client_id, zoneMsg);

    // Nuevo terreno + actores/items de la nueva zona
    GameMsg mapMsg(MSG_SEND_MAP);
    mapMsg.set_map(game_map.get_map(name));
    client_registry_monitor.notify_client(client_id, mapMsg);

    send_npcs_snapshot_to(client_id);
    send_items_snapshot_to(client_id);
    // El que llega recibe el snapshot de players de la zona destino (antes no se
    // mandaba: por eso no veía a los que ya estaban ahí).
    send_players_snapshot_to(client_id, name);
    // Y los que ya estaban en la zona destino se enteran del recién llegado (sin
    // esto solo lo verían cuando se mueva).
    send_player_snapshot_to_other_players(client_id, name, game_map.get_player(name).get_race_name());
}

void GameLoop::handle_move(const ClientCmd& cmd) {
    std::string name = client_registry_monitor.get_name(cmd.get_client_id());
    auto result = game_map.try_move(cmd.get_direction(), name);
    // if (result.moved) {
        if (game_map.pick_up_gold(name)) {
            std::cout << "oro" << std::endl;
            broadcast_items_snapshot();  // todos ven que el oro desapareció del piso
            GameMsg msg_gold(MSG_GOLD);
            msg_gold.set_gold(game_map.get_player_gold(name));
            // El oro recogido es solo del que lo levantó: notificar SOLO a él.
            // Con notify_clients, el resto de los HUD pisaban su propio oro con este valor.
            client_registry_monitor.notify_client(cmd.get_client_id(), msg_gold);
        }
        GameMsg msg(MSG_MOVE, cmd.get_direction());
        msg.set_player_name(result.player_name);
        msg.set_coord_x(result.new_x);
        msg.set_coord_y(result.new_y);
        msg.set_race(cmd.get_race());
        std::cout << "[DEBUG: handle_move] player " << cmd.get_player_name()
              << " has race " << cmd.get_race() << std::endl;
        // Solo los de la MISMA zona ven el movimiento (incluido el propio mover,
        // que usa su MSG_MOVE para fijar su posición/POV).
        notify_zone(game_map.get_player_zone(name), msg);
        std::cout << "[DEBUG]: sended" << std::endl;
    // }

    // Teleport automatico: si el player se movio y quedo parado sobre una celda
    // teleport, se cambia de zona (mismo flujo que el cheat /tp)
    // MSG_ZONE_CHANGE lo reubica en la zona destino
    if (result.moved) {
        auto tp = game_map.try_teleport_on_current_cell(name);
        if (tp.on_tile) {
            send_zone_transition(cmd.get_client_id(), name, tp);
        }
    }
}

void GameLoop::handle_attack(const ClientCmd& cmd) {
    int x = cmd.get_coord_x();
    int y = cmd.get_coord_y();
    std::string attacker_name = client_registry_monitor.get_name(cmd.get_client_id());
    try {
        auto result = game_map.attack(attacker_name, x, y);
        // notifica solo al atacante que su ataque impactó esa celda (no se hace
        // broadcast). Se envía siempre que el ataque ocurrió: el cliente lo usa
        // para mostrar el número de daño y animar el efecto del hechizo sobre el
        // target. (target_x/target_y = celda atacada).
        GameMsg dmg_msg(MSG_ATTACK);
        dmg_msg.set_coord_x(result.target_x);
        dmg_msg.set_coord_y(result.target_y);
        dmg_msg.set_damage(result.damage);
        client_registry_monitor.notify_client(cmd.get_client_id(), dmg_msg);

        if (result.level != 0) {
            GameMsg level_msg(MSG_UPDATE_LEVEL);
            level_msg.set_level(result.level);
            level_msg.set_max_xp(game_map.player_max_xp(attacker_name));
            // Subir de nivel aumenta la vida/maná maximos: que el HUD actualice el
            // tope de las barras (sin esto se veria vida actual > vida maxima).
            level_msg.set_max_hp(game_map.get_player_max_hp(attacker_name));
            level_msg.set_max_mana(game_map.get_player_max_mana(attacker_name));
            client_registry_monitor.notify_client(cmd.get_client_id(), level_msg);
        }
        // Feed de combate (estilo AO) en el minichat del ATACANTE: cuánto daño
        // provocó, o si el target esquivó. Viaja como MSG_CHAT.
        GameMsg atk_chat(MSG_CHAT);
        if (result.dodged) {
            atk_chat.set_chat_content(result.entity_name + " ha esquivado tu ataque");
        } else {
            atk_chat.set_chat_content("Le has provocado " + std::to_string(result.damage) +
                                      " de daño a " + result.entity_name);
        }
        client_registry_monitor.notify_client(cmd.get_client_id(), atk_chat);

        if (result.entity_died) {
            broadcast_npcs_snapshot();
            broadcast_items_snapshot();
            GameMsg death_msg(MSG_DEATH);
            death_msg.set_player_name(result.entity_name);
            client_registry_monitor.notify_clients(death_msg);
        }
        // El atacante gana XP en cada golpe: notificarle su XP actualizada
        GameMsg xp_msg(MSG_XP);
        xp_msg.set_xp(game_map.get_player_xp(attacker_name));
        client_registry_monitor.notify_client(cmd.get_client_id(), xp_msg);

        // Si el ataque fue con un hechizo, consumió maná: notificar el actualizado.
        GameMsg mana_msg(MSG_MANA);
        mana_msg.set_player_name(attacker_name);
        mana_msg.set_mana(game_map.get_player_mana(attacker_name));
        client_registry_monitor.notify_client(cmd.get_client_id(), mana_msg);
        if (result.target_is_player) {
            GameMsg hp_msg(MSG_HP);
            hp_msg.set_hp(game_map.get_player_hp(result.entity_name));
            client_registry_monitor.notify_client_by_name(result.entity_name, hp_msg);

            // Feed de combate en el minichat de la VÍCTIMA: cuánto daño recibió y
            // de quién, o si logró esquivar. Notifica al otro cliente por nombre.
            GameMsg victim_chat(MSG_CHAT);
            if (result.dodged) {
                victim_chat.set_chat_content("¡Has esquivado el ataque de " + attacker_name + "!");
            } else {
                victim_chat.set_chat_content(attacker_name + " te ha provocado " +
                                             std::to_string(result.damage) + " de daño");
            }
            client_registry_monitor.notify_client_by_name(result.entity_name, victim_chat);

            if (result.entity_died) {
                GameMsg gold_msg(MSG_GOLD);
                gold_msg.set_gold(game_map.get_player_gold(result.entity_name));
                client_registry_monitor.notify_client_by_name(result.entity_name, gold_msg);
                const Player& p = game_map.get_player(result.entity_name);
                std::vector<ItemInfo> item_infos;
                for (Item* item : p.get_all_items()) {
                    item_infos.emplace_back(item->get_id(), item->getName(), item->getPrice(), static_cast<uint8_t>(item->get_type()), item->get_uid());
                }
                GameMsg inv_msg(MSG_INVENTORY);
                inv_msg.set_items(item_infos);
                client_registry_monitor.notify_client_by_name(result.entity_name, inv_msg);
                broadcast_player_equipment(result.entity_name);
            }
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
    } catch (const CannotCastException& e) {
        GameMsg msg(MSG_CHAT);
        msg.set_chat_content(e.what());
        client_registry_monitor.notify_client(cmd.get_client_id(), msg);
    } catch (const NotEnoughManaException& e) {
        GameMsg msg(MSG_CHAT);
        msg.set_chat_content(e.what());
        client_registry_monitor.notify_client(cmd.get_client_id(), msg);
    }
}

void GameLoop::handle_meditate(const ClientCmd& cmd) {
    std::string name = client_registry_monitor.get_name(cmd.get_client_id());
    Player* player = game_map.get_player_mut(name);
    if (player == nullptr) return;

    bool meditating = player->toggle_meditation();

    GameMsg msg(MSG_MEDITATE);
    if (meditating) {
        msg.set_chat_content("Estas meditando");
    } else if (player->can_meditate()) {
        msg.set_chat_content("Has dejado de meditar");
    } else {
        // No pudo empezar: o es guerrero (no medita) o está muerto.
        msg.set_chat_content("No podés meditar.");
    }
    client_registry_monitor.notify_client(cmd.get_client_id(), msg);

    std::cout << "[INFO: MSG_MEDITATE] " << name
              << (meditating ? " empezo a meditar" : " no esta meditando") << std::endl;
}

void GameLoop::handle_self_cast(const ClientCmd& cmd) {
    std::string name = client_registry_monitor.get_name(cmd.get_client_id());
    try {
        game_map.self_cast(name);
        // El hechizo (p.ej. curación) cambió vida y maná: notificar al jugador.
        GameMsg hp_msg(MSG_HP);
        hp_msg.set_hp(game_map.get_player_hp(name));
        client_registry_monitor.notify_client(cmd.get_client_id(), hp_msg);

        GameMsg mana_msg(MSG_MANA);
        mana_msg.set_player_name(name);
        mana_msg.set_mana(game_map.get_player_mana(name));
        client_registry_monitor.notify_client(cmd.get_client_id(), mana_msg);
    } catch (const NoWeaponEquippedException& e) {
        GameMsg msg(MSG_CHAT);
        msg.set_chat_content(e.what());
        client_registry_monitor.notify_client(cmd.get_client_id(), msg);
    } catch (const CannotCastException& e) {
        GameMsg msg(MSG_CHAT);
        msg.set_chat_content(e.what());
        client_registry_monitor.notify_client(cmd.get_client_id(), msg);
    } catch (const NotEnoughManaException& e) {
        GameMsg msg(MSG_CHAT);
        msg.set_chat_content(e.what());
        client_registry_monitor.notify_client(cmd.get_client_id(), msg);
    }
}

void GameLoop::handle_use_item(const ClientCmd& cmd) {
    std::string name = client_registry_monitor.get_name(cmd.get_client_id());
    // El cliente manda el uid de INSTANCIA del item a usar (en texto decimal),
    // igual que MSG_EQUIP.
    uint64_t item_uid = 0;
    try {
        item_uid = std::stoull(cmd.get_item_id());
    } catch (const std::exception&) {
        return;  // uid mal formado: ignorar el comando
    }

    bool consumed = game_map.use_item(name, item_uid);
    if (!consumed) return;  // uid inexistente o item no consumible: nada que notificar

    // La poción curó vida y/o maná: notificar los valores nuevos.
    GameMsg hp_msg(MSG_HP);
    hp_msg.set_hp(game_map.get_player_hp(name));
    client_registry_monitor.notify_client(cmd.get_client_id(), hp_msg);

    GameMsg mana_msg(MSG_MANA);
    mana_msg.set_player_name(name);
    mana_msg.set_mana(game_map.get_player_mana(name));
    client_registry_monitor.notify_client(cmd.get_client_id(), mana_msg);

    // La poción se consumió: reenviar el inventario para que desaparezca del HUD.
    const Player& p = game_map.get_player(name);
    std::vector<ItemInfo> item_infos;
    for (Item* item : p.get_all_items()) {
        item_infos.emplace_back(item->get_id(), item->getName(), item->getPrice(),
                                static_cast<uint8_t>(item->get_type()), item->get_uid());
    }
    GameMsg inv_msg(MSG_INVENTORY);
    inv_msg.set_items(item_infos);
    client_registry_monitor.notify_client(cmd.get_client_id(), inv_msg);
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
    game_map.kill_player(name);
    GameMsg msg(MSG_CHEAT_KILL);
    msg.set_player_name(name);
    msg.set_chat_content(name + " murió instantáneamente.");
    client_registry_monitor.notify_clients(msg);
    broadcast_items_snapshot();
    GameMsg gold_msg(MSG_GOLD);
    gold_msg.set_gold(game_map.get_player_gold(name));
    client_registry_monitor.notify_client_by_name(name, gold_msg);
    const Player& p = game_map.get_player(name);
    std::vector<ItemInfo> item_infos;
    for (Item* item : p.get_all_items()) {
        item_infos.emplace_back(item->get_id(), item->getName(), item->getPrice(), static_cast<uint8_t>(item->get_type()));
    }
    GameMsg inv_msg(MSG_INVENTORY);
    inv_msg.set_items(item_infos);
    client_registry_monitor.notify_client_by_name(name, inv_msg);
    broadcast_player_equipment(name);
}

void GameLoop::handle_cheat_inf_hp(const ClientCmd& cmd) {
    std::string name = client_registry_monitor.get_name(cmd.get_client_id());
    game_map.set_infinite_hp(name);
    GameMsg msg(MSG_CHAT);
    msg.set_chat_content("Vida infinita activada.");
    client_registry_monitor.notify_client(cmd.get_client_id(), msg);
}

void GameLoop::handle_cheat_inf_mana(const ClientCmd& cmd) {
    std::string name = client_registry_monitor.get_name(cmd.get_client_id());
    game_map.set_infinite_mana(name);
    GameMsg msg(MSG_CHAT);
    msg.set_chat_content("Mana infinito activado.");
    client_registry_monitor.notify_client(cmd.get_client_id(), msg);
}

void GameLoop::handle_cheat_mana(const ClientCmd& cmd) {
    std::string name = client_registry_monitor.get_name(cmd.get_client_id());
    // La cantidad viaja en el campo gold del ClientCmd (cantidad genérica).
    game_map.cheat_lose_mana(name, cmd.get_gold());

    // Notifica el maná actualizado al HUD.
    GameMsg msg(MSG_MANA);
    msg.set_player_name(name);
    msg.set_mana(game_map.get_player_mana(name));
    client_registry_monitor.notify_client(cmd.get_client_id(), msg);

    std::cout << "[INFO: MSG_CHEAT_MANA] " << name << " perdio "
              << cmd.get_gold() << " de mana (queda "
              << game_map.get_player_mana(name) << ")" << std::endl;
}

void GameLoop::handle_cheat_revive(const ClientCmd& cmd) {
    std::string name = client_registry_monitor.get_name(cmd.get_client_id());
    game_map.revive_player(name);

    // Avisa a todos para que dejen de dibujar al jugador como fantasma.
    GameMsg msg(MSG_CHEAT_RESPAWN);
    msg.set_player_name(name);
    msg.set_chat_content(name + " resucitó.");
    client_registry_monitor.notify_clients(msg);

    // Sincroniza el HUD del propio jugador: revivir restaura vida/maná al máximo
    // y resetea la experiencia.
    GameMsg hp_msg(MSG_HP);
    hp_msg.set_player_name(name);
    hp_msg.set_hp(game_map.get_player_hp(name));
    client_registry_monitor.notify_client(cmd.get_client_id(), hp_msg);

    GameMsg mana_msg(MSG_MANA);
    mana_msg.set_player_name(name);
    mana_msg.set_mana(game_map.get_player_mana(name));
    client_registry_monitor.notify_client(cmd.get_client_id(), mana_msg);

    GameMsg xp_msg(MSG_XP);
    xp_msg.set_player_name(name);
    xp_msg.set_xp(game_map.get_player_xp(name));
    client_registry_monitor.notify_client(cmd.get_client_id(), xp_msg);
}

void GameLoop::handle_clan_foundation(const ClientCmd& cmd) {
    std::string player_name = client_registry_monitor.get_name(cmd.get_client_id());
    std::string clan_name = cmd.get_target_name();
    GameMsg clan_msg(MSG_FOUND_CLAN);
    if (!game_map.found_clan(player_name, clan_name)) {
        clan_msg.set_chat_content("Ya existe un clan con el nombre: " + clan_name);
        client_registry_monitor.notify_client(cmd.get_client_id(), clan_msg);
    } else {
        clan_msg.set_chat_content("Jugador " + player_name + " fundó el clan " + clan_name);
        client_registry_monitor.notify_clients(clan_msg);
        persist_clans();
    }
}

void GameLoop::handle_clan_joining(const ClientCmd& cmd) {
    std::string player_name = client_registry_monitor.get_name(cmd.get_client_id());
    std::string clan_name = cmd.get_target_name();
    GameMsg clan_msg(MSG_JOIN_CLAN);
    if (!game_map.join_clan(player_name, clan_name)) {
        clan_msg.set_chat_content("No podes solicitar unirte al clan: " + clan_name);
    } else {
        clan_msg.set_chat_content("Solicitud de unión al clan " + clan_name + " enviada");
        // join_request anexa la solicitud al review del clan: persistir el cambio.
        persist_clans();
    }
    client_registry_monitor.notify_client(cmd.get_client_id(), clan_msg);
}

void GameLoop::handle_clan_reviewing(const ClientCmd& cmd) {
    std::string player_name = client_registry_monitor.get_name(cmd.get_client_id());
    std::string result = game_map.rev_clan(player_name);
    GameMsg clan_msg(MSG_REV_CLAN);
    clan_msg.set_chat_content(result);
    client_registry_monitor.notify_client(cmd.get_client_id(), clan_msg);
}

void GameLoop::handle_clan_accepting(const ClientCmd& cmd) {
    std::string player_name = client_registry_monitor.get_name(cmd.get_client_id());
    std::string new_member = cmd.get_target_name();

    GameMsg clan_msg(MSG_CLAN_ACEP);
    game_map.accept_new_member(player_name, new_member);
    clan_msg.set_chat_content("Jugador " + new_member + " fue aceptado a unirse al clan fundado por " + player_name);
    client_registry_monitor.notify_clients(clan_msg);
    persist_clans();
}

void GameLoop::handle_clan_rejecting(const ClientCmd& cmd) {
    std::string player_name = client_registry_monitor.get_name(cmd.get_client_id());
    std::string new_member = cmd.get_target_name();

    GameMsg clan_msg(MSG_CLAN_RECH);
    clan_msg.set_chat_content("Jugador " + new_member + " fue rechazado a unirse al clan fundado por " + player_name);
    client_registry_monitor.notify_clients(clan_msg);
}

void GameLoop::handle_clan_leaving(const ClientCmd& cmd) {
    std::string player_name = client_registry_monitor.get_name(cmd.get_client_id());

    GameMsg personal_msg(MSG_LEFT_CLAN);
    GameMsg clan_msg(MSG_LEFT_CLAN);
    std::string clan_name;
    if (!game_map.leave_clan(player_name, clan_name)) {
        personal_msg.set_chat_content("No podes abandonar el clan " + clan_name);
        client_registry_monitor.notify_client(cmd.get_client_id(), personal_msg);
        return;
    }
    clan_msg.set_chat_content("Jugador " + player_name + " abandonó el clan " + clan_name);
    client_registry_monitor.notify_clients(clan_msg);
    persist_clans();
}

void GameLoop::handle_clan_kick(const ClientCmd& cmd) {
    std::string player_name = client_registry_monitor.get_name(cmd.get_client_id());
    std::string member = cmd.get_target_name();

    GameMsg personal_msg(MSG_CLAN_KICK);
    GameMsg clan_msg(MSG_CLAN_KICK);
    
    if (!game_map.kick_member(player_name, member)) {
        personal_msg.set_chat_content("El jugador " + member + " no es parte del clan");
        client_registry_monitor.notify_client(cmd.get_client_id(), personal_msg);
        return;
    }
    clan_msg.set_chat_content("Jugador " + member + " fue echado del clan");
    client_registry_monitor.notify_clients(clan_msg);
    persist_clans();
}

void GameLoop::handle_clan_ban(const ClientCmd& cmd) {
    std::string player_name = client_registry_monitor.get_name(cmd.get_client_id());
    std::string member = cmd.get_target_name();

    GameMsg personal_msg(MSG_CLAN_BAN);
    GameMsg clan_msg(MSG_CLAN_BAN);
    
    if (!game_map.ban_member(player_name, member)) {
        personal_msg.set_chat_content("El jugador " + member + " no es parte del clan");
        client_registry_monitor.notify_client(cmd.get_client_id(), personal_msg);
        return;
    }
    clan_msg.set_chat_content("Jugador " + member + " fue banneado del clan");
    client_registry_monitor.notify_clients(clan_msg);
    persist_clans();
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
    const auto& cfg = GameConfig::instance();
    const auto tick_rate = std::chrono::milliseconds(cfg.tick_rate_ms);// 20 ticks/s
    auto next_tick = std::chrono::steady_clock::now();

    // Acumulamos ticks para resolver la regeneracion (maná al meditar) una vez
    // por segundo: evita spamear MSG_MANA cada 50ms.
    int ticks_accumulated = 0;
    const int ticks_per_second = cfg.ticks_per_second; // 1000ms / 50ms
    int npc_move_ticks = 0;
    const int TICKS_PER_NPC_MOVE = 10;

    // Regeneracion automatica de vida: fraccion de la vida maxima que cada
    // player vivo recupera por segundo real (0.01 = 1%).
    const double LIFE_REGEN_PER_SECOND = 0.01;
    const double MANA_REGEN_PER_SECOND = 0.01;

    while (should_keep_running()) {
        next_tick += tick_rate;

        try {
            ClientCmd cmd;
            while (receiving_queue.try_pop(cmd)) {
                process_cmd(cmd);
            }
            update_npcs_in_map();
            if (++npc_move_ticks >= TICKS_PER_NPC_MOVE) {
                npc_move_ticks = 0;
                auto npc_attacks = game_map.update_npc_aggro();
                for (const auto& attack : npc_attacks) {
                    GameMsg hp_msg(MSG_HP);
                    hp_msg.set_hp(game_map.get_player_hp(attack.victim_name));
                    client_registry_monitor.notify_client_by_name(attack.victim_name, hp_msg);

                    GameMsg victim_chat(MSG_CHAT);
                    if (attack.dodged) {
                        victim_chat.set_chat_content("¡Has esquivado el ataque de un " + attack.npc_name + "!");
                    } else {
                        victim_chat.set_chat_content("Un " + attack.npc_name + " te ha provocado " +
                                                    std::to_string(attack.damage) + " de daño");
                    }
                    client_registry_monitor.notify_client_by_name(attack.victim_name, victim_chat);

                    if (attack.victim_died) {
                        game_map.kill_player(attack.victim_name);
                        broadcast_npcs_snapshot();
                        broadcast_items_snapshot();
                        GameMsg death_msg(MSG_DEATH);
                        death_msg.set_player_name(attack.victim_name);
                        client_registry_monitor.notify_clients(death_msg);
                        GameMsg gold_msg(MSG_GOLD);
                        gold_msg.set_gold(game_map.get_player_gold(attack.victim_name));
                        client_registry_monitor.notify_client_by_name(attack.victim_name, gold_msg);
                        const Player& p = game_map.get_player(attack.victim_name);
                        std::vector<ItemInfo> item_infos;
                        for (Item* item : p.get_all_items()) {
                            item_infos.emplace_back(item->get_id(), item->getName(), item->getPrice(), static_cast<uint8_t>(item->get_type()));
                        }
                        GameMsg inv_msg(MSG_INVENTORY);
                        inv_msg.set_items(item_infos);
                        client_registry_monitor.notify_client_by_name(attack.victim_name, inv_msg);
                        broadcast_player_equipment(attack.victim_name);
                    }

                }
                broadcast_npcs_snapshot();
            }

            if (++ticks_accumulated >= ticks_per_second) {
                ticks_accumulated = 0;
                regen_players_mana(1.0);  // maná de meditación (escala con inteligencia)
                // Regeneracion automatica (pasiva) cada segundo real: todos los
                // players vivos recuperan un porcentaje de su vida y maná maximos.
                regen_players_life(LIFE_REGEN_PER_SECOND);
                regen_players_mana_passive(MANA_REGEN_PER_SECOND);
            }

            // Guardado periodico: red de seguridad ante desconexiones abruptas.
            if (++tick_count % PERSIST_INTERVAL_TICKS == 0) {
                persist_online_players();
                persist_clans();
            }
        }
        catch (... ) {
            //
        }
        std::this_thread::sleep_until(next_tick);
    }
}

void GameLoop::persist_online_players() {
    for (const auto& [client_id, name] : client_registry_monitor.get_active_clients()) {
        (void)client_id;
        if (name.empty() || !game_map.player_exists(name)) continue;
        // La password ya esta persistida en el record previo; la recuperamos para
        // no perderla al reescribir (el Player no la conoce).
        PlayerRecord prev;
        std::string password;
        if (persistence.load(name, prev)) {
            password = std::string(prev.password,
                                   ::strnlen(prev.password, PERSIST_PASSWORD_MAX));
        }
        persistence.save(name, player_serializer.to_record(
                game_map.get_player(name), game_map.get_player_zone(name), password));
    }
}

void GameLoop::load_persisted_clans() {
    std::vector<Clan> clans;
    for (const ClanRecord& rec : clan_persistence.load_all()) {
        clans.push_back(clan_serializer.from_record(rec));
    }
    game_map.load_clans(std::move(clans));
}

void GameLoop::persist_clans() {
    std::vector<ClanRecord> records;
    for (const auto& [name, clan] : game_map.get_clans()) {
        (void)name;
        records.push_back(clan_serializer.to_record(clan));
    }
    clan_persistence.save_all(records);
}

void GameLoop::regen_players_mana(double seconds) {
    for (const std::string& name : game_map.tick(seconds)) {
        GameMsg msg(MSG_MANA);
        msg.set_player_name(name);
        msg.set_mana(game_map.get_player_mana(name));
        client_registry_monitor.notify_client_by_name(name, msg);
    }
}

void GameLoop::regen_players_life(double percent) {
    // Cura 'percent' de la vida maxima a cada player vivo y notifica el HP nuevo
    // solo a los que cambiaron (los que ya estaban al maximo no entran en la lista).
    for (const std::string& name : game_map.regen_all_players_life(percent)) {
        GameMsg msg(MSG_HP);
        msg.set_player_name(name);
        msg.set_hp(game_map.get_player_hp(name));
        client_registry_monitor.notify_client_by_name(name, msg);
    }
}

void GameLoop::regen_players_mana_passive(double percent) {
    // Cura 'percent' del maná maximo a cada player vivo y notifica el maná nuevo
    // solo a los que cambiaron (los que ya estaban al maximo no entran en la lista).
    for (const std::string& name : game_map.regen_all_players_mana(percent)) {
        GameMsg msg(MSG_MANA);
        msg.set_player_name(name);
        msg.set_mana(game_map.get_player_mana(name));
        client_registry_monitor.notify_client_by_name(name, msg);
    }
}
