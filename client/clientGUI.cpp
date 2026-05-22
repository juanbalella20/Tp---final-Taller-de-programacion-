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
        tilemap = std::make_unique<TileMap>(renderer);
        tilemap->load("data/maps/desert/map.toml");
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

void ClientGUI::sendCoord(int x, int y) {
    ClientCmd cmd;
    cmd.set_message_type(MSG_SELECT);
    cmd.set_coord_x(x);
    cmd.set_coord_y(y);
    outgoing.push(cmd);
}


void ClientGUI::handleEvents() {
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_EVENT_QUIT:
                is_running = false;
                break;
            case SDL_EVENT_KEY_DOWN:
                if (event.key.repeat) break;
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
            case SDL_EVENT_MOUSE_BUTTON_DOWN:
                sendCoord(event.button.x, event.button.y);
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
            int x = player->getTileX();
            int y = player->getTileY();
            switch (msg.get_direction()) {
                case DIR_NORTH: --y; break;
                case DIR_SOUTH: ++y; break;
                case DIR_EAST:  ++x; break;
                case DIR_WEST:  --x; break;
                default: break;
            }
            player->setTilePosition(x, y);
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
    if (tilemap) {
        tilemap->render();
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
    int tileSize = tilemap->getTileSize();
    try {
        player = std::make_unique<PlayerDisplay>(renderer, "images/player.png", tileSize);
        std::cout << "[DEBUG] PlayerDisplay created (images/)" << std::endl;
    } catch (const std::runtime_error& e) {
        std::cout << "[DEBUG] images/player.png failed: " << e.what()
                  << " - trying imagenes/" << std::endl;
        player = std::make_unique<PlayerDisplay>(renderer, "imagenes/player.png", tileSize);
        std::cout << "[DEBUG] PlayerDisplay created (imagenes/)" << std::endl;
    }

    // Posicionar al player en el spawn "player_start" del TOML.
    if (tilemap) {
        std::cout << "[DEBUG] spawns count=" << tilemap->getSpawns().size() << std::endl;
        bool found = false;
        for (const auto& sp : tilemap->getSpawns()) {
            std::cout << "[DEBUG] spawn: name='" << sp.name
                      << "' x=" << sp.x << " y=" << sp.y << std::endl;
            if (sp.name == "player_start") {
                player->setTilePosition(sp.x, sp.y);
                std::cout << "[DEBUG] player positioned at ("
                          << sp.x << "," << sp.y << ")" << std::endl;
                found = true;
                break;
            }
        }
        if (!found) {
            std::cout << "[DEBUG] player_start NOT FOUND, defaulting to (1,1)" << std::endl;
            player->setTilePosition(1, 1);
        }
    } else {
        std::cout << "[DEBUG] no tilemap, player at default (0,0)" << std::endl;
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
