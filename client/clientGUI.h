#ifndef CLIENT_GUI_H
#define CLIENT_GUI_H

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <string>
#include <memory>
#include <vector>
#include "minichat.h"
#include "displayPlayer.h"
#include "parser.h"
#include "../common/thread.h"
#include "../common/queue.h"
#include "../common/clientCmd.h"
#include "../common/gameMsg.h"
#include "../common/game_constants.h"
#include "../common/item_info.h"
#include "../common/npc_info.h"
#include "tileMap.h"
#include "camera.h"
#include "hud.h"

#define WIN_NAME "Argentum"
#define WINDOW_WIDTH 1280
#define WINDOW_HEIGHT 720

#define LOGICAL_WIDTH  960
#define LOGICAL_HEIGHT 576

enum class zones {
    DESERT,
    FOREST,
    CITY
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

    std::unique_ptr<PlayerDisplay> player;
    std::unique_ptr<TileMap> tilemap;

    std::vector<std::vector<elements>> world_map;
    std::vector<NpcInfo> npcs;
    SDL_Texture* enemy_texture;
    SDL_Texture* frame_texture;
    SDL_Texture* item_texture;
    SDL_Texture* gold_texture;
    Camera camera;

    // tile seleccionado con NPC; {-1,-1} = ninguno
    int selected_npc_tile_x;
    int selected_npc_tile_y;

    static constexpr int PANEL_WIDTH  = 192;
    static constexpr int GAME_WIDTH   = LOGICAL_WIDTH - PANEL_WIDTH;
    static constexpr int CANVAS_HEIGHT = LOGICAL_HEIGHT;

    void initSDL();
    void loadMedia(zones zone);
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

    void sendMoveCmd(Direction dir);
    void sendAttackCmd(int tile_x, int tile_y);
    void sendEquipCmd(const std::string& item_id);
    void sendChatCmd(const std::string& msg);

    // recibe mensaje del server y hace el dibujo inicial
    void init_draw();
    //PRE: SE RECIBEM LAS COORDENADAS DE DONDE ESTAN
    //POS ESAS COORDENADAS SE ENVIAN AL SERVIDOR PARA QUE SE MUEVA EL JUGADOR A ESA POS
    void sendCoord(int x, int y);

    // Verifica si hay NPC en la celda clickeada y muestra el botón "Pegar"
    void selectCoord(int tile_x, int tile_y);

    std::vector<int> translate_tile_to_coord(int pixel_x, int pixel_y) const;

    void set_logical_width(int logical_width);
    void set_logical_height(int logical_height);

public:
    ClientGUI(Queue<ClientCmd>& outgoing, Queue<GameMsg>& receiving);
    ~ClientGUI();

    ClientGUI(const ClientGUI&) = delete;
    ClientGUI& operator=(const ClientGUI&) = delete;

    void run() override;
};

#endif
