#ifndef CLIENT_GUI_H
#define CLIENT_GUI_H

#include <SDL3/SDL.h>
#include <string>
#include <memory>
#include "displayPlayer.h"
#include "../common/thread.h"
#include "../common/queue.h"
#include "../common/clientCmd.h"
#include "../common/gameMsg.h"
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

    void initSDL();
    void loadMedia(zones zone);
    void freeSDL();

    void handleEvents();
    void update();
    void draw();

    void sendMoveCmd(Direction dir);

    // recibe mensaje del server y hace el dibujo inicial
    void init_draw();

    //PRE: SE RECIBEM LAS COORDENADAS DE DONDE ESTAN
    //POS ESAS COORDENADAS SE ENVIAN AL SERVIDOR PARA QUE SE MUEVA EL JUGADOR A ESA POS
    void sendCoord(int x, int y);

public:
    ClientGUI(Queue<ClientCmd>& outgoing, Queue<GameMsg>& receiving);
    ~ClientGUI();

    ClientGUI(const ClientGUI&) = delete;
    ClientGUI& operator=(const ClientGUI&) = delete;

    void run() override;
};

#endif
