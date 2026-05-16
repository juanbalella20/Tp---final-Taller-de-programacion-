#include "clientGUI.h"
#include <SDL3_image/SDL_image.h>
#include <iostream>
#include <stdexcept>

ClientGUI::ClientGUI(Queue<ClientCmd>& outgoing, Queue<GameMsg>& receiving)
    : window(nullptr), renderer(nullptr), background(nullptr), event{},
      is_running(false), outgoing(outgoing), receiving(receiving), player(nullptr) {}

ClientGUI::~ClientGUI() {
    freeSDL();
}

void ClientGUI::initSDL() {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        throw std::runtime_error(std::string("SDL_Init: ") + SDL_GetError());
    }
    window = SDL_CreateWindow(WIN_NAME, WINDOW_WIDTH, WINDOW_HEIGHT, 0);
    if (!window) {
        throw std::runtime_error(std::string("SDL_CreateWindow: ") + SDL_GetError());
    }
    renderer = SDL_CreateRenderer(window, nullptr);
    if (!renderer) {
        throw std::runtime_error(std::string("SDL_CreateRenderer: ") + SDL_GetError());
    }

    SDL_Surface* icon = IMG_Load("images/logo.jpeg");
    if (icon) {
        SDL_SetWindowIcon(window, icon);
        SDL_DestroySurface(icon);
    }
}

void ClientGUI::loadMedia() {
    background = IMG_LoadTexture(renderer, "images/background.jpg");
    if (!background) {
        throw std::runtime_error(std::string("Loading background: ") + SDL_GetError());
    }
}

void ClientGUI::freeSDL() {
    player.reset();

    if (background) {
        SDL_DestroyTexture(background);
        background = nullptr;
    }
    if (renderer) {
        SDL_DestroyRenderer(renderer);
        renderer = nullptr;
    }
    if (window) {
        SDL_DestroyWindow(window);
        window = nullptr;
    }
    SDL_Quit();
}

void ClientGUI::handleEvents() {
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_EVENT_QUIT:
                is_running = false;
                break;
            case SDL_EVENT_KEY_DOWN:
                switch (event.key.scancode) {
                    case SDL_SCANCODE_ESCAPE:
                        is_running = false;
                        break;
                    case SDL_SCANCODE_UP:
                    case SDL_SCANCODE_W:
                        sendMoveCmd(DIR_NORTH);
                        break;
                    case SDL_SCANCODE_DOWN:
                    case SDL_SCANCODE_S:
                        sendMoveCmd(DIR_SOUTH);
                        break;
                    case SDL_SCANCODE_RIGHT:
                    case SDL_SCANCODE_D:
                        sendMoveCmd(DIR_EAST);
                        break;
                    case SDL_SCANCODE_LEFT:
                    case SDL_SCANCODE_A:
                        sendMoveCmd(DIR_WEST);
                        break;
                    default:
                        break;
                }
                break;
            default:
                break;
        }
    }
}

void ClientGUI::sendMoveCmd(Direction dir) {
    ClientCmd cmd;
    cmd.set_message_type(MSG_MOVE);
    cmd.set_direction(dir);
    outgoing.push(cmd);
}

void ClientGUI::update() {
    player->update();
}

void ClientGUI::draw() {
    SDL_RenderClear(renderer);
    SDL_RenderTexture(renderer, background, nullptr, nullptr);
    player->draw();
    SDL_RenderPresent(renderer);
}

void ClientGUI::run() {
    try {
        initSDL();
        loadMedia();
        player = std::make_unique<PlayerDisplay>(renderer, "images/player.png");
        is_running = true;

        SDL_Delay(100);
        while (is_running && should_keep_running()) {
            handleEvents();
            update();
            draw();
            SDL_Delay(16);
        }
    } catch (const std::exception& e) {
        std::cerr << "ClientGUI error: " << e.what() << std::endl;
    }
}
