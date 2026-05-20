#ifndef CLIENT_GUI_H
#define CLIENT_GUI_H

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <string>
#include <memory>
#include "minichat.h"
#include "displayPlayer.h"
#include "../common/thread.h"
#include "../common/queue.h"
#include "../common/clientCmd.h"
#include "../common/gameMsg.h"


#define WIN_NAME "Argentum"
#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

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

    Queue<ClientCmd>& outgoing;
    Queue<GameMsg>& receiving;
    Queue<std::string> chat_inbox;

    std::unique_ptr<PlayerDisplay> player;
    //PlayerDisplay& player;

    void initSDL();
    void loadMedia(zones zone);
    void freeSDL();

    void handleEvents();
    void update();
    void draw();

    void sendMoveCmd(Direction dir);
    void sendChatCmd(const std::string& msg);

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
