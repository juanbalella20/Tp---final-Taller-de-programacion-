#ifndef GAME_MAP_H_
#define GAME_MAP_H_

#include "../../common/commands/clientCmd.h"
#include "player/player.h"
#include "npc/npcHostile.h"
#include "../../common/constants/game_constants.h"
#include "../../common/constants/protocol_constants.h"
#include "../../common/info/item_info.h"
#include "../../common/info/npc_info.h"
#include "../../common/info/item_floor_info.h"
#include "../../common/mapLoader.h"
#include "npc/npcSeller.h"
#include "item/item.h"
#include "zoneWorld.h"
#include "player/clan.h"

#include <vector>
#include <map>
#include <string>
#include <utility>
#include "playerinfo.h"

struct NpcSpawn {
    std::string type;
    int x;
    int y;
};

struct ItemSpawn {
    std::string type;
    int x;
    int y;
};

struct InitialState {
    int num_npc;
    int num_items;
};

struct TeleportResult {
        bool on_tile;
        Zone dest_zone;
        int x;
        int y;
        // Zona desde la que se fue el player (para avisar a esa zona que se retiró).
        // Solo es válida si on_tile == true.
        Zone src_zone = ZONE_DEFAULT;
};

// Factory: construye un NPChostile a partir de un NpcSpawn
NPChostile make_npc_from_spawn(const NpcSpawn& spawn);


class GameMap {

private:
    std::map<Zone, ZoneWorld> zones;          // enum Zone -> mundo de esa zona
    std::vector<Player> players;              // players globales (atraviesan zonas)
    std::map<std::string, Zone> player_zone;  // player_name -> zona actual (tag)
    std::map<std::string, Clan> clans;

    Player* find_player_by_name(const std::string& name);

    // Helpers de orquestacion: resuelven la zona de un player
    ZoneWorld& zone_of(const std::string& player_name);
    Zone zone_id_of(const std::string& player_name) const;
    // Punteros (const) a los players que estan en la zona z
    std::vector<Player*> players_in(Zone z);

    // Celda donde aparece un player que llega a la zona dst: adyacente al
    // teleport de esa zona si tiene, o una celda libre random. {-1,-1} si nada.
    std::pair<int, int> find_arrival_cell(ZoneWorld& dst, Zone dest_zone);

    // Recibe una Zona y su mundo, elige un tipo de NPC permitido en esa zona y
    // genera un NPChostile random ubicado en una celda libre. Si la zona no
    // tiene tipos permitidos, devuelve un NPC en {-1,-1} (no se spawnea).
    NPChostile rand_npc(Zone zone, ZoneWorld& world);

    // Construye un item concreto elegido al azar. Se devuelve por unique_ptr
    // porque Item es una interfaz abstracta.
    std::unique_ptr<Item> rand_item();
public:
    GameMap();

    // Mapa (matriz de terreno) de la zona donde está el player
    std::vector<std::vector<elements>> get_map(const std::string& player_name);

    void add_player(Player player);

    // Saca al jugador del mundo (lo borra de players y de su tag de zona). Se usa
    // al desconectarse. No-op si el jugador no existe.
    void remove_player(const std::string& name);

    // Alta de un jugador cargado de persistencia: ademas de agregarlo, lo etiqueta
    // en su zona persistida (add_player solo no setea player_zone). Reusa el tag
    // de zona que hace spawn_player.
    void add_persisted_player(Player player, Zone zone);

    // Devuelve un snapshot de todos los jugadores en el mismo mapa que player_name
    // (excluyéndolo a él). Hoy todo está en "desert"; cuando haya múltiples mapas,
    // filtrar por sector aquí.
    std::vector<PlayerInfo> build_players_snapshot(const std::string& player_name);
    struct MoveResult {
        bool moved;
        std::string player_name;
        int new_x;
        int new_y;
    };

    struct AttackResult {
        bool hit;
        bool entity_died;
        bool target_is_player;
        std::string entity_name;
        int damage;
        int target_x;
        int target_y;
        bool dodged;  // el target esquivó el golpe (solo PvP)
        int level;
    };

    // Calcula la nueva posicion del player a partir de su posicion actual y la
    // direccion, resolviendo limites/terreno/actores contra SU zona
    MoveResult try_move(Direction dir, const std::string& player_name);

    // Ataca la celda (x,y) en la zona del atacante
    AttackResult attack(const std::string& atacker_name, int x, int y);

    // El jugador lanza el hechizo del item equipado sobre sí mismo (auto-cast,
    // p.ej. curación). Propaga las excepciones de Baculo::use_item.
    void self_cast(const std::string& player_name);

    // El jugador toma/consume un item de su inventario por uid (p.ej. una poción):
    // aplica su efecto (cura) y lo saca del inventario. Devuelve true si se
    // consumió (false si el uid no existe o el item no es consumible).
    bool use_item(const std::string& player_name, uint64_t item_uid);

    // Respawn de NPCs en TODAS las zonas. Devuelve true si hubo alguno
    bool update_npcs();

    std::vector<NPCAttackEvent> update_npc_aggro();

    // Avance de tiempo: hace tick() en todos los players. Devuelve los nombres
    // de los que están meditando (su maná cambió: hay que notificarles MSG_MANA).
    std::vector<std::string> tick(double seconds);

    // Regeneracion automatica de vida: cura un 'percent' (0..1) de la vida
    // maxima a todos los players vivos. Devuelve los nombres de los que
    // cambiaron de vida (hay que notificarles MSG_HP).
    std::vector<std::string> regen_all_players_life(double percent);

    std::vector<std::string> regen_all_players_mana(double percent);

    // Snapshots de la zona del player indicado
    std::vector<NpcInfo> build_npcs_snapshot(const std::string& player_name);
    std::vector<ItemFloorInfo> build_items_snapshot(const std::string& player_name);

    // Comercio: el seller se ubica por (x,y) en la zona del player
    bool player_sell_item(const std::string& player_name, int x, int y, const std::string& item_id);
    bool player_buy_item(const std::string& player_name, int x, int y, const std::string& item_id);
    std::vector<ItemInfo> list_seller_items(const std::string& player_name, int x, int y);

    // Zona actual del player (para MSG_ZONE_CHANGE)
    Zone get_player_zone(const std::string& player_name) const;


    // Fuerza el cambio del jugador a 'dest_zone',
    // ubicandolo en una celda libre de llegada. Devuelve {false,...} si la zona no esta
    // cargada o no hay celda libre.
    TeleportResult force_zone_change(const std::string& player_name, Zone dest_zone);

    // Equipa/desequipa (toggle) el item del jugador, identificado por su uid de
    // instancia (distingue dos copias del mismo tipo).
    // Devuelve true si, tras la operación, el jugador tiene un arma equipada.
    bool player_equip_item(const std::string& player_name, uint64_t item_uid);
    void spawn_player(const std::string& name, const std::string& race, const std::string& pclass);
    const Player& get_player(const std::string& name);
    // Acceso mutable por nombre para handlers que cambian estado del player
    // (p. ej. meditar). Devuelve nullptr si no existe.
    Player* get_player_mut(const std::string& name);
    bool player_exists(const std::string& name);

    std::unique_ptr<Item> pick_up_item(const std::string& player_name);
    void give_item_to_player(const std::string& player_name, std::unique_ptr<Item> item);

    uint32_t get_player_gold(const std::string& name);
    uint32_t get_player_hp(const std::string& name);
    uint32_t get_player_xp(const std::string& name);
    uint32_t player_max_xp(const std::string& name);
    uint32_t get_player_mana(const std::string& name);
    uint32_t get_player_max_hp(const std::string& name);
    uint32_t get_player_max_mana(const std::string& name);

    // Cheat /mana: resta `amount` de maná al player (para testear /meditar).
    void cheat_lose_mana(const std::string& name, uint32_t amount);

    void set_infinite_hp(std::string player_name);
    void set_infinite_mana(std::string player_name);

    // Cheat /cheat-resucitar: revive al player si está muerto (fantasma),
    // restaurando vida/maná al máximo.
    void revive_player(const std::string& player_name);
    /*
     * Carga TODAS las zonas al iniciar el server. Por cada (zone_id, path):
     * crea la ZoneWorld, carga su terreno y spawnea sus actores segun el
     * InitialState correspondiente.
     */
    void init_world(const std::map<Zone, std::string>& zone_paths,
                    const std::map<Zone, InitialState>& initial_states);

    bool pick_up_gold(const std::string& player_name);

    void kill_player(const std::string& player_name);
    
    bool found_clan(const std::string& player_name, const std::string& clan_name);
    bool join_clan(const std::string& player_name, const std::string& clan_name);
    std::string rev_clan(const std::string& player_name);
    void accept_new_member(const std::string& player_name, const std::string& new_member);
    bool leave_clan(const std::string& player_name, std::string& clan_name);
    bool kick_member(const std::string& player_name, const std::string& member);
    bool ban_member(const std::string& player_name, const std::string& member);

    bool same_clan(Player* player1, Player* player2);

    // --- Persistencia de clanes ---
    // Acceso de solo lectura al set de clanes vivos (para volcarlos a disco).
    const std::map<std::string, Clan>& get_clans() const { return clans; }
    // Reemplaza el set de clanes por los cargados de disco al iniciar el server.
    void load_clans(std::vector<Clan> persisted_clans);

    // Busca al player y su zona, comprueba que esté parado en una celda de teleport
    TeleportResult try_teleport_on_current_cell(const std::string& player_name);
};

#endif
