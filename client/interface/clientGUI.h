#ifndef CLIENT_GUI_H
#define CLIENT_GUI_H

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <string>
#include <memory>
#include <vector>
#include <map>
#include <set>
#include "minichat.h"
#include "displayPlayer.h"
#include "parser.h"
#include "thread.h"
#include "queue.h"
#include "clientCmd.h"
#include "gameMsg.h"
#include "game_constants.h"
#include "item_info.h"
#include "npc_info.h"
#include "item_floor_info.h"
#include "playerinfo.h"
#include "tileMap.h"
#include "camera.h"
#include "hud.h"
#include "shopWindow.h"
#include "zone_music_player.h"
#include "sound_player.h"
#include "../../common/utility/thread.h"

#define WIN_NAME "Argentum"
#define WINDOW_WIDTH 1280
#define WINDOW_HEIGHT 720

#define LOGICAL_WIDTH  1024
#define LOGICAL_HEIGHT 576
#define DMG_MS 800

// Numero de daño flotante que aparece sobre una celda y desaparece.
struct DamageNumber {
    int tile_x;
    int tile_y;
    int value;
    uint64_t expire_ms;
};

// Catalogo de un efecto visual de hechizo: spritesheet animado por frames.
struct SpellEffectDef {
    SDL_Texture* texture = nullptr;
    int frame_w = 0;       // ancho de cada frame en el spritesheet
    int frame_h = 0;       // alto de cada frame
    int cols = 0;          // columnas de la grilla
    int frame_count = 0;   // cantidad total de frames a reproducir
};

// Animacion de hechizo en curso, dibujada sobre una celda y avanzando frames.
struct SpellAnimation {
    int tile_x;
    int tile_y;
    std::string effect_id;   // clave en spell_effects (id del baculo)
    uint64_t start_ms;
};

class ClientGUI: public Thread {
private:
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Event event;
    TTF_Font* chat_font;
    bool is_running;
    std::unique_ptr<MiniChat> mini_chat;
    Parser parser;
    Queue<ClientCmd>& outgoing;
    Queue<GameMsg>& receiving;
    Queue<std::string> chat_inbox;
    std::unique_ptr<HUD> hud;
    // Ventana modal de comercio (la tienda del vendedor). Se abre al recibir
    // MSG_LIST de un seller (comando /listar adyacente). Solo lectura.
    std::unique_ptr<ShopWindow> shop_window;
    std::string own_name;  // nombre del jugador local
    std::string race;      // raza del jugador local
    std::string own_clan;  // clan del jugador local ("" si no tiene): decide el
                           // color del nombre de los demas (verde mismo clan / rojo)

    std::unique_ptr<PlayerDisplay> player;
    std::unique_ptr<TileMap> tilemap;
    std::unique_ptr<ZoneMusicPlayer> zone_music;
    std::unique_ptr<SoundPlayer> sfx;

    std::vector<std::vector<elements>> world_map;
    std::vector<NpcInfo> npcs;
    std::vector<ItemFloorInfo> items_on_floor;
    std::vector<PlayerInfo> other_players;
    std::unordered_map<std::string, SDL_FRect> other_players_povs;
    // Sprite persistente de cada jugador ajeno, por nombre. Construir un
    // PlayerDisplay carga ~20 PNGs de disco y crea sus texturas: debe hacerse
    // UNA vez por jugador y reusarse entre frames (hacerlo por frame quemaba
    // el CPU apenas habia otro jugador en la zona).
    std::map<std::string, PlayerDisplay> other_player_displays;
    std::map<std::string, SDL_Texture*> enemies_textures;
    
    std::vector<DamageNumber> damage_numbers;

    // --- Flash rojo en los bordes al recibir daño (feedback inmersivo) ---
    // Cada vez que el HP local baja, se arranca un "destello": una viñeta roja
    // difuminada en los bordes del area de juego que se desvanece sola. Para
    // detectar la baja guardamos el ultimo HP conocido (los MSG_HP traen el
    // valor absoluto, no el delta).
    uint32_t last_known_hp = 0;       // HP local del ultimo MSG_HP/REGISTER
    bool has_known_hp = false;        // ¿ya recibimos un HP inicial?
    uint64_t damage_flash_until_ms = 0;  // SDL_GetTicks hasta el que dura el flash
    static constexpr uint64_t DAMAGE_FLASH_MS = 450;  // duracion del destello
    // Efectos de hechizo: catalogo (id de baculo -> spritesheet) y animaciones activas.
    std::map<std::string, SpellEffectDef> spell_effects;
    std::vector<SpellAnimation> spell_animations;
    // Id del baculo actualmente equipado por el jugador local ("" si ninguno o si
    // tiene un arma fisica). Sirve para deducir que efecto animar al castear.
    std::string equipped_spell_id;
    // Id (clave del config: "espada", "baculo_nudoso", ...) del arma/baculo de
    // ataque equipado por el jugador local. Sirve para elegir el sonido al atacar.
    std::string equipped_weapon_id;
    // Uids (de INSTANCIA, en texto) de los items de defensa equipados en el ultimo
    // MSG_UPDATE_EQUIP. Sirve para detectar cuando se equipa una defensa NUEVA y
    // reproducir el sonido de equipacion solo en ese caso (no al desequipar).
    std::set<std::string> equipped_defense_uids;
    SDL_Texture* frame_texture;
    SDL_Texture* item_texture;
    SDL_Texture* gold_texture;
    // Textura y recorte del sprite de cada item del piso, por id de item.
    std::map<std::string, SDL_Texture*> floor_item_textures;
    std::map<std::string, SDL_FRect> floor_item_crops;

    // --- Tomar una poción manteniendo el click IZQUIERDO 3s sobre su slot ---
    // Mientras se mantiene presionado, se dibuja un anillo de progreso alrededor
    // del slot; al completarse los 3s se manda MSG_USE_ITEM y la poción se consume.
    static constexpr float POTION_HOLD_MS = 3000.0f;  // 3 segundos
    bool potion_hold_active = false;        // ¿hay un hold en curso?
    bool potion_hold_sent = false;          // ¿ya se mandó el use (evita repetir)?
    uint64_t potion_hold_uid = 0;           // uid de instancia de la poción presionada
    uint64_t potion_hold_start_ms = 0;      // SDL_GetTicks al empezar el hold
    SDL_FRect potion_hold_slot_rect = {0, 0, 0, 0};  // rect en pantalla del slot

    SDL_Texture* seller_texture;
    SDL_Texture* banker_texture;
    SDL_Texture* priest_texture;
    Camera camera;
    SDL_FRect player_pov;
    Zone current_zone;

    // tile seleccionado con NPC; {-1,-1} = ninguno
    int selected_npc_tile_x;
    int selected_npc_tile_y;

    static constexpr int PANEL_WIDTH  = 276;
    static constexpr int GAME_WIDTH   = LOGICAL_WIDTH - PANEL_WIDTH;
    static constexpr int CANVAS_HEIGHT = LOGICAL_HEIGHT;

    // Recorte transparente ("hueco") del frame en_ventanaprincipal.png donde se
    // ve el mundo, en coordenadas logicas.
    static constexpr int GAME_VIEW_X = 12;
    static constexpr int GAME_VIEW_Y = 113;
    static constexpr int GAME_VIEW_W = 732;
    static constexpr int GAME_VIEW_H = 456;

    // Inicializa el estado grafico de ClientGUI sobre el window/renderer/font
    // COMPARTIDOS (propiedad del ScreenManager): fija la presentacion logica,
    // el icono y el mini chat. No crea ni destruye window/renderer/TTF.
    void initSDL();
    void loadMedia(Zone zone);
    void freeSDL();

    /*
    Por cada evento del teclado, primero se fija si es del MiniChat.
    Si sí, no hace nada más. Si no, mueve al personaje.
    Después, le pregunta al MiniChat si hay un mensaje nuevo.
    Si sí, lo obtiene y llama a sendChatCmd()
    */
    void handleEvents();

    /*
    Lee los mensajes que llegan del servidor.
    Si llegó uno del chat, lo guarda en chat_inbox.
    Luego, llama al update de MiniChat.
    */
    void update();

    /*
    Llama a lo último al render de MiniChat para que el chat
    se dibuje por encima de todo lo anterior dibujado.
    */
    void draw();
    void drawEnemies();
    void drawItems();
    void drawOtherPlayers();

    void sendMoveCmd(Direction dir);
    void sendAttackCmd(int tile_x, int tile_y);
    void sendSelfCastCmd();
    void sendEquipCmd(const std::string& item_id);
    void sendUseItemCmd(const std::string& item_uid);
    void sendChatCmd(const std::string& msg);

    // Devuelve true si el slot que cae bajo (mx,my) en el inventario es una
    // poción (item consumible); si lo es, deja su uid en out_uid y el rect del
    // slot en pantalla en out_rect. Reusa la misma geometría que el equipar.
    bool potion_slot_at(int mx, int my, uint64_t& out_uid, SDL_FRect& out_rect) const;
    // Avanza el estado del hold de poción: si se cumplieron los 3s, manda el use.
    void update_potion_hold();
    // Dibuja el anillo de progreso del hold sobre el slot presionado.
    void draw_potion_hold_arc();

    // recibe mensaje del server y hace el dibujo inicial
    void init_draw();
    //PRE: SE RECIBEM LAS COORDENADAS DE DONDE ESTAN
    //POS ESAS COORDENADAS SE ENVIAN AL SERVIDOR PARA QUE SE MUEVA EL JUGADOR A ESA POS
    void sendCoord(int x, int y);

    // Si hay NPC en la celda: ataca directamente. Si no: mueve al jugador.
    void selectCoord(int tile_x, int tile_y);

    std::vector<int> translate_tile_to_coord(int pixel_x, int pixel_y) const;

    void set_logical_width(int logical_width);
    void set_logical_height(int logical_height);
    void draw_npc_friends();
    // Dibuja "Transportarse a <zona>" sobre cada tile de teleport del mapa.
    void draw_teleport_labels();
    // Dibuja los numeros de daño flotantes en rojo y descarta los expirados.
    void draw_damage_numbers();

    // Registra que el HP local cambio a new_hp: si bajo respecto al ultimo
    // conocido, arranca el flash rojo de daño. Llamar en cada MSG_HP.
    void note_local_hp(uint32_t new_hp);
    // Dibuja la viñeta roja difuminada en los bordes del area de juego mientras
    // el flash este activo. La intensidad decae con el tiempo restante.
    void draw_damage_flash();

    // Carga los spritesheets de efectos de hechizo (uno por baculo).
    void load_spell_effects();
    // Dispara una animacion de efecto del baculo dado sobre la celda (tile_x,tile_y).
    void spawn_spell_animation(const std::string& effect_id, int tile_x, int tile_y);
    // Dibuja y avanza las animaciones de hechizo activas.
    void draw_spell_animations();

    // Ruta (relativa a assets) del sonido del arma/báculo cuyo id es weapon_id,
    // o nullptr si esa arma no tiene sonido asignado.
    static const char* weapon_sound_path(const std::string& weapon_id);
    void load_npc_texture(const std::string& npc_name, const std::string& image_path);
    void load_enemies_textures();

    // Dibuja el nombre de un jugador centrado sobre su sprite. tile_x/tile_y son
    // las coords en celdas; el color lo decide el clan (verde mismo clan / rojo).
    void draw_player_name(const std::string& name, int tile_x, int tile_y,
                          SDL_Color color);

public:
    // window/renderer/font son COMPARTIDOS (propiedad del ScreenManager): se
    // reciben, no se crean ni se destruyen aca.
    ClientGUI(SDL_Window* window, SDL_Renderer* renderer, TTF_Font* font,
              Queue<ClientCmd>& outgoing, Queue<GameMsg>& receiving,
              const std::string& player_name, const std::string& player_race,
              const std::string& player_clan = "");
    ~ClientGUI();

    ClientGUI(const ClientGUI&) = delete;
    ClientGUI& operator=(const ClientGUI&) = delete;

    void run() override;
};

#endif
