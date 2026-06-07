#ifndef SERVER_SRC_GAMELOOP_H_
#define SERVER_SRC_GAMELOOP_H_

#include <functional>
#include <unordered_map>

#include "../common/commands/clientCmd.h"
#include "../common/utility/queue.h"
#include "../common/utility/thread.h"
#include "clientRegistryMonitor.h"
#include "game_map.h"
#include "persistence/player_persistence.h"
#include "persistence/player_serializer.h"
#include "persistence/auth_service.h"

class GameLoop : public Thread {
 public:
    GameLoop(Queue<ClientCmd>& receiving_queue, ClientRegistryMonitor& client_registry_monitor);
    void run() override;

 private:
    Queue<ClientCmd>& receiving_queue;
    ClientRegistryMonitor& client_registry_monitor;
    GameMap game_map;

    // Persistencia: dos archivos binarios (records + indice). El game loop es el
    // unico hilo que muta el mundo, asi que persiste sin locks.
    PlayerPersistence persistence;
    PlayerSerializer player_serializer;
    AuthService auth;
    uint64_t tick_count = 0;  // para el guardado periodico (PERSIST_INTERVAL_TICKS)

    // client_id -> {x, y} del ultimo NPC seleccionado
    std::unordered_map<uint32_t, std::pair<int, int>> selected_npc;

    using Handler = std::function<void(const ClientCmd&)>;
    std::unordered_map<uint8_t, Handler> handlers;

    void register_handlers();
    void load_maps();
    void load_world();
    void update_npcs_in_map();
    // Regenera maná de los players que meditan y les notifica MSG_MANA.
    void regen_players_mana(double seconds);
    // Busca en la matriz de IntitialState (previamente parseada desde config.TOML)
    // y devulve el estado inicial por zona: cantidad de npcs, cantidad de items que genera esa zona
    InitialState load_initial_state_from_file(Zone zone);
    // Version hardcodeada. TODO: eliminar cuando este implementada la persistencia
    InitialState load_initial_state_hardcoded(Zone zone);
    void broadcast_npcs_snapshot();
    void send_npcs_snapshot_to(uint32_t client_id);
    void broadcast_items_snapshot();
    void send_items_snapshot_to(uint32_t client_id);
    void send_players_snapshot_to(uint32_t client_id, const std::string& player_name);
    void send_player_snapshot_to_other_players(uint32_t client_id, const std::string& player_name, 
         const std::string& player_race);

    void process_cmd(const ClientCmd& cmd);

    // Envia un error de auth (register/login fallido) al cliente.
    void send_auth_error(uint32_t client_id, const std::string& reason);
    // Arma y envia todo el estado inicial del mundo al cliente recien autenticado
    // (mapa, stats, items, snapshots de NPCs/players, zona). Compartido por
    // register y login.
    void send_world_snapshot_to(uint32_t client_id, const std::string& name,
                                const std::string& race);
    // Persiste a disco el estado de todos los jugadores online (guardado periodico).
    void persist_online_players();

    void handle_register(const ClientCmd& cmd);
    void handle_login(const ClientCmd& cmd);
    void handle_list(const ClientCmd& cmd);
    void handle_sell(const ClientCmd& cmd);
    void handle_buy(const ClientCmd& cmd);
    void handle_equip(const ClientCmd& cmd);
    void handle_select(const ClientCmd& cmd);
    void handle_take(const ClientCmd& cmd);
    void handle_move(const ClientCmd& cmd);
    void handle_attack(const ClientCmd& cmd);
    void handle_meditate(const ClientCmd& cmd);
    void handle_self_cast(const ClientCmd& cmd);
    void handle_teleport(const ClientCmd& cmd);
    void send_zone_transition(uint32_t client_id, const std::string& name,
                              const TeleportResult& r);
    void handle_private(const ClientCmd& cmd);
    void handle_cheat_kill(const ClientCmd& cmd);
    void handle_cheat_inf_hp(const ClientCmd& cmd);
    void handle_cheat_inf_mana(const ClientCmd& cmd);
    void handle_cheat_mana(const ClientCmd& cmd);
    void handle_clan_foundation(const ClientCmd& cmd);
    void handle_clan_joining(const ClientCmd& cmd);
    void handle_clan_reviewing(const ClientCmd& cmd);
    void handle_clan_accepting(const ClientCmd& cmd);
    void handle_clan_rejecting(const ClientCmd& cmd);
    void handle_clan_leaving(const ClientCmd& cmd);
    void handle_clan_kick(const ClientCmd& cmd);
    void handle_clan_ban(const ClientCmd& cmd);
};

#endif  // SERVER_SRC_GAMELOOP_H_
