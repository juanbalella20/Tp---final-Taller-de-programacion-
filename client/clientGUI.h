#ifndef CLIENT_GUI_H
#define CLIENT_GUI_H

#include <SDL3/SDL.h>
#include <string>
#include <memory>
#include "displayPlayer.h"
#include "../common/thread.h"
#include "../common/queue.h"
#include "../common/clientCmd.h"

class GameMsg;  // eliminar cuando se mergee 

#define WIN_NAME "Argentum"
#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

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

    void initSDL();
    void loadMedia();
    void freeSDL();

    void handleEvents();
    void update();
    void draw();

public:
    ClientGUI(Queue<ClientCmd>& outgoing, Queue<GameMsg>& receiving);
    ~ClientGUI();

    ClientGUI(const ClientGUI&) = delete;
    ClientGUI& operator=(const ClientGUI&) = delete;

    void run() override;
};

#endif
