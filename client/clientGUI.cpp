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

    SDL_SetRenderLogicalPresentation(renderer, LOGICAL_WIDTH, LOGICAL_HEIGHT,SDL_LOGICAL_PRESENTATION_LETTERBOX);

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
        tilemap->load_map("data/maps/desert/map.toml");
        break;
    }
    default:
        break;
    }
}

void ClientGUI::freeSDL() {
    player.reset();
    tilemap.reset();

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
    // tile_size viene del TOML
    // el player se escala con el mismo tamano de celda
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
    // (posicionar al player en el spawn que indica el server!)
    if (tilemap) {
        const auto& spawns = tilemap->getSpawns();
        std::cout << "[DEBUG] spawns count=" << spawns.size() << std::endl;
        // hardcodeado desde el .toml
        auto it = spawns.find("player_start");
        if (it != spawns.end()) {
            player->setTilePosition(it->second.x, it->second.y);
            // debug print!
            std::cout << "[DEBUG] player positioned at ("
                      << it->second.x << "," << it->second.y << ")" << std::endl;
        } else {
            // debug print!
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
