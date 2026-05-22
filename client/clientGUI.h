#ifndef CLIENT_GUI_H
#define CLIENT_GUI_H

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <string>
#include <memory>
#include "minichat.h"
#include "displayPlayer.h"
#include "parser.h"
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
    TTF_Font* chat_font;
    bool is_running;
    std::unique_ptr<MiniChat> mini_chat;
    Parser parser;
    Queue<ClientCmd>& outgoing;
    Queue<GameMsg>& receiving;
    Queue<std::string> chat_inbox;

    std::unique_ptr<PlayerDisplay> player;
    //PlayerDisplay& player;
    std::unique_ptr<TileMap> tilemap;

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

    void sendMoveCmd(Direction dir);
    void sendChatCmd(const std::string& msg);

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
