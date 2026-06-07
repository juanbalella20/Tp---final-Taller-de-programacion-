#ifndef CLIENT_GUI_H
#define CLIENT_GUI_H

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <string>
#include <memory>
#include <vector>
#include <map>
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

struct WindowSettings;
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
    std::string own_name;  // nombre del jugador local
    std::string race;      // raza del jugador local

    std::unique_ptr<PlayerDisplay> player;
    std::unique_ptr<TileMap> tilemap;

    std::vector<std::vector<elements>> world_map;
    std::vector<NpcInfo> npcs;
    std::vector<ItemFloorInfo> items_on_floor;
    std::vector<PlayerInfo> other_players;
    SDL_Texture* enemy_texture;
    std::map<std::string, SDL_Texture*> enemies_textures;
    // Recorte del 1er tile del spritesheet por tipo de NPC (nombre -> src crop)
    // TODO: refactorizar para permitir movimientos de los npcs
    std::map<std::string, SDL_FRect> enemies_crops;
    std::vector<DamageNumber> damage_numbers;
    // Efectos de hechizo: catalogo (id de baculo -> spritesheet) y animaciones activas.
    std::map<std::string, SpellEffectDef> spell_effects;
    std::vector<SpellAnimation> spell_animations;
    // Id del baculo actualmente equipado por el jugador local ("" si ninguno o si
    // tiene un arma fisica). Sirve para deducir que efecto animar al castear.
    std::string equipped_spell_id;
    SDL_Texture* frame_texture;
    SDL_Texture* item_texture;
    SDL_Texture* gold_texture;
    // Textura y recorte del sprite de cada item del piso, por id de item.
    std::map<std::string, SDL_Texture*> floor_item_textures;
    std::map<std::string, SDL_FRect> floor_item_crops;
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

    void initSDL(const WindowSettings& settings);
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
    void sendChatCmd(const std::string& msg);

    // recibe mensaje del server y hace el dibujo inicial
    void init_draw(const WindowSettings& settings);
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

    // Carga los spritesheets de efectos de hechizo (uno por baculo).
    void load_spell_effects();
    // Dispara una animacion de efecto del baculo dado sobre la celda (tile_x,tile_y).
    void spawn_spell_animation(const std::string& effect_id, int tile_x, int tile_y);
    // Dibuja y avanza las animaciones de hechizo activas.
    void draw_spell_animations();

public:
    ClientGUI(Queue<ClientCmd>& outgoing, Queue<GameMsg>& receiving, const std::string& player_name, const std::string& player_race);
    ~ClientGUI();

    ClientGUI(const ClientGUI&) = delete;
    ClientGUI& operator=(const ClientGUI&) = delete;

    void run() override;
};

#endif
