#include "clientGUI.h"
#include <SDL3_image/SDL_image.h>
#include <array>
#include <iostream>
#include <stdexcept>

ClientGUI::ClientGUI(Queue<ClientCmd>& outgoing, Queue<GameMsg>& receiving)
    : window(nullptr), renderer(nullptr), background(nullptr), event{},
      is_running(false), outgoing(outgoing), receiving(receiving) {}

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
    if (!icon) {
        icon = IMG_Load("imagenes/logo.jpeg");
    }
    if (icon) {
        SDL_SetWindowIcon(window, icon);
        SDL_DestroySurface(icon);
    }
}

// tiene que recibir los 4 sectorPerimiter y mostrar solo eso
void ClientGUI::loadMedia(zones zone) {
    switch (zone)
    {
    case zones::DESERT : {
        constexpr std::array<const char*, 2> bg_paths = {
            "images/background.jpg",
            "imagenes/background.jpg"
        };
        for (const char* path : bg_paths) {
            background = IMG_LoadTexture(renderer, path);
            if (background) {
                break;
            }
        }
        if (!background) {
            throw std::runtime_error(std::string("Loading background: ") + SDL_GetError());
        }
        break;
    }
    default:
        break;
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
    try {
        if (!player) {
            return;
        }
        GameMsg msg(0);
        while (receiving.try_pop(msg)) {
            if (msg.get_type() != MSG_MOVE) {
                continue;
            }
            switch (msg.get_direction()) {
                case DIR_NORTH:
                    player->move_up();
                    break;
                case DIR_SOUTH:
                player->move_down();
                    break;
                case DIR_EAST:
                    player->move_right();
                    break;
                case DIR_WEST:
                    player->move_left();
                    break;
            }
        }
    } catch (const ClosedQueue&) {
        is_running = false;
    }
}

void ClientGUI::draw() {
    SDL_RenderClear(renderer);
    if (background) {
        SDL_RenderTexture(renderer, background, nullptr, nullptr);
    }
    if (player) {
        player->draw();
    }
    SDL_RenderPresent(renderer);
}

void ClientGUI::init_draw() {
    // la info se la pasa el cliente desde config?
    initSDL(); 
    // aca recibe del protocolo la zona
    // hardocodeado para test
    loadMedia(zones::DESERT);
    try {
        player = std::make_unique<PlayerDisplay>(renderer, "images/player.png");
    } catch (const std::runtime_error&) {
        player = std::make_unique<PlayerDisplay>(renderer, "imagenes/player.png");
    }
    is_running = true;
    SDL_Delay(100);

}

void ClientGUI::run() {
    try {
        // protocol.get_mapa();
        init_draw();

        while (is_running && should_keep_running()) {
            handleEvents();
            update();
            draw();
            SDL_Delay(16);
        }
    } catch (const std::exception& e) {
        std::cerr << "ClientGUI error: " << e.what() << std::endl;
    }

    freeSDL();
}
