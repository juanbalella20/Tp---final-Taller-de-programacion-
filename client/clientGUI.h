#ifndef CLIENT_GUI_H
#define CLIENT_GUI_H

#include <SDL3/SDL.h>
#include <string>
#include <memory>
#include <vector>
#include "displayPlayer.h"
#include "../common/thread.h"
#include "../common/queue.h"
#include "../common/clientCmd.h"
#include "../common/gameMsg.h"
#include "../common/game_constants.h"
#include "tileMap.h"


#define WIN_NAME "Argentum"
#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
//#define WINDOW_WIDTH 1920
//#define WINDOW_HEIGHT 1080

enum class zones {
    DESERT,
    FOREST,
    CITY
};

class ClientGUI: public Thread {
private:
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Texture* background;
    SDL_Event event;
    bool is_running;

    Queue<ClientCmd>& outgoing;
    Queue<GameMsg>& receiving;

    std::unique_ptr<PlayerDisplay> player;
     //PlayerDisplay& player;
    std::unique_ptr<TileMap> tilemap;

    std::vector<std::vector<elements>> world_map;
    SDL_Texture* enemy_texture;

    // tile seleccionado con NPC; {-1,-1} = ninguno
    int selected_npc_tile_x;
    int selected_npc_tile_y;
    bool show_attack_button;

    void initSDL();
    void loadMedia(zones zone);
    void freeSDL();

    void handleEvents();
    void update();
    void draw();
    void drawEnemies();
    void drawAttackButton();

    void sendMoveCmd(Direction dir);
    void sendAttackCmd(int tile_x, int tile_y);
    //PRE: SE RECIBEM LAS COORDENADAS DE DONDE ESTAN
    //POS ESAS COORDENADAS SE ENVIAN AL SERVIDOR PARA QUE SE MUEVA EL JUGADOR A ESA POS
    void sendCoord(int x, int y);

    // Verifica si hay NPC en la celda clickeada y muestra el botón "Pegar"
    void selectCoord(int tile_x, int tile_y);

    std::vector<int> translate_tile_to_coord(int pixel_x, int pixel_y) const;
 // recibe mensaje del server y hace el dibujo inicial
    void init_draw();

public:
    ClientGUI(Queue<ClientCmd>& outgoing, Queue<GameMsg>& receiving);
    ~ClientGUI();

    ClientGUI(const ClientGUI&) = delete;
    ClientGUI& operator=(const ClientGUI&) = delete;

    void run() override;
};

#endif
