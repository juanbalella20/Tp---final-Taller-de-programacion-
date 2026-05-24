#include "clientGUI.h"
#include <SDL3_image/SDL_image.h>
#include <iostream>
#include <stdexcept>

ClientGUI::ClientGUI(Queue<ClientCmd>& outgoing, Queue<GameMsg>& receiving)
    : window(nullptr), renderer(nullptr), event{}, chat_font(nullptr),
      is_running(false), mini_chat(nullptr), parser(), outgoing(outgoing), receiving(receiving),
      enemy_texture(nullptr), selected_npc_tile_x(-1), selected_npc_tile_y(-1),
      show_attack_button(false) {}
    

ClientGUI::~ClientGUI() {
    freeSDL();
    TTF_CloseFont(chat_font);
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

    if (!TTF_Init() == -1) {
        throw std::runtime_error(std::string("TTF_Init: ") + SDL_GetError());
    }

    chat_font = TTF_OpenFont("fonts/Roboto-VariableFont_wdth,wght.ttf", 16);

    mini_chat = std::make_unique<MiniChat>(renderer, chat_font);
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


    if (enemy_texture) {
        SDL_DestroyTexture(enemy_texture);
        enemy_texture = nullptr;
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

std::vector<int> ClientGUI::translate_tile_to_coord(int pixel_x, int pixel_y) const {
    int tileSize = tilemap ? tilemap->getTileSize() : 64;
    return {pixel_x / tileSize, pixel_y / tileSize};
}

void ClientGUI::sendAttackCmd(int tile_x, int tile_y) {
    ClientCmd cmd;
    cmd.set_message_type(MSG_ATTACK);
    cmd.set_coord_x(tile_x);
    cmd.set_coord_y(tile_y);
    outgoing.push(cmd);
}

void ClientGUI::sendCoord(int tile_x, int tile_y) {
    ClientCmd cmd;
    cmd.set_message_type(MSG_SELECT);
    cmd.set_coord_x(tile_x);
    cmd.set_coord_y(tile_y);
    outgoing.push(cmd);
}

void ClientGUI::selectCoord(int tile_x, int tile_y) {
    if (!world_map.empty() &&
        tile_y >= 0 && tile_y < static_cast<int>(world_map.size()) &&
        tile_x >= 0 && tile_x < static_cast<int>(world_map[tile_y].size()) &&
        world_map[tile_y][tile_x] == elements::npcs) {
        selected_npc_tile_x = tile_x;
        selected_npc_tile_y = tile_y;
        show_attack_button = true;
    } else {
        show_attack_button = false;
        selected_npc_tile_x = -1;
        selected_npc_tile_y = -1;
        sendCoord(tile_x, tile_y);
    }
}


void ClientGUI::handleEvents() {
    while (SDL_PollEvent(&event)) {
        
        if (mini_chat->handle_event(event)) {
            continue;
        }

        switch (event.type) {
            case SDL_EVENT_QUIT:
                is_running = false;
                break;
            case SDL_EVENT_KEY_DOWN:
                if (event.key.scancode == SDL_SCANCODE_T) {
                    if (!(mini_chat->is_active())) {
                        mini_chat->toggle_active();
                    }
                    break;
                }
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
            case SDL_EVENT_MOUSE_BUTTON_DOWN: {
                int mx = static_cast<int>(event.button.x);
                int my = static_cast<int>(event.button.y);
                // Click en el botón "Pegar" (panel derecho, parte inferior)
                SDL_FRect btn = {static_cast<float>(GAME_WIDTH + 10),
                                 static_cast<float>(WINDOW_HEIGHT - 60),
                                 static_cast<float>(PANEL_WIDTH - 20), 40};
                if (show_attack_button &&
                    mx >= btn.x && mx <= btn.x + btn.w &&
                    my >= btn.y && my <= btn.y + btn.h) {
                    sendAttackCmd(selected_npc_tile_x, selected_npc_tile_y);
                    show_attack_button = false;
                } else {
                    auto coords = translate_tile_to_coord(mx, my);
                    selectCoord(coords[0], coords[1]);
                }
                break;
            }
            default:
                break;
        }
    }

    if (mini_chat->has_pending_outbound_message()) {
        std::string msg = mini_chat->pop_outbound_message();
        sendChatCmd(msg);
    }
}

void ClientGUI::sendMoveCmd(Direction dir) {
    ClientCmd cmd;
    cmd.set_message_type(MSG_MOVE);
    cmd.set_direction(dir);
    outgoing.push(cmd);
}

void ClientGUI::sendChatCmd(const std::string& msg) {
    try {
        ClientCmd cmd = parser.parse_chat(msg);
        outgoing.push(cmd);
    } catch (const std::invalid_argument& e) {
        chat_inbox.push(std::string("Error: ") + e.what());
    }
}

void ClientGUI::update() {
    try {
        // static bool tested = false;
        // if (!tested) {
        //     chat_inbox.push("Hola");
        //     tested = true;
        // }
        if (!player) {
            return;
        }
        GameMsg msg(0);
        while (receiving.try_pop(msg)) {
            std::cout << "Mesaje recibido tipo: " << (int)msg.get_type() << std::endl;
            switch (msg.get_type()) {
                case MSG_SEND_MAP:
                    world_map = msg.get_map();
                    break;
                case MSG_INVENTORY:
                    inventory = msg.get_items();
                    break;
                case MSG_MOVE: {
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
                    break;
                }
                case MSG_PRIVATE:
                case MSG_MEDITATE:
                case MSG_RESURRECT:
                case MSG_CURE:
                case MSG_LIST:
                case MSG_BUY:
                case MSG_SELL:
                case MSG_DEPOSIT:
                case MSG_RETIRE:
                case MSG_DEP_GOLD:
                case MSG_RET_GOLD:
                case MSG_FOUND_CLAN:
                case MSG_JOIN_CLAN:
                case MSG_REV_CLAN:
                case MSG_CLAN_ACEP:
                case MSG_CLAN_BAN:
                case MSG_CLAN_KICK:
                case MSG_CLAN_RECH:
                case MSG_LEFT_CLAN:
                case MSG_CHAT:
                case MSG_CHEAT_KILL:
                case MSG_CHEAT_INF_HP:
                case MSG_CHEAT_INF_MANA:
                    chat_inbox.push(msg.get_chat_content());
                    break;
                default:
                    break;
            }
        }

        mini_chat->update(chat_inbox);

    } catch (const ClosedQueue&) {
        is_running = false;
    }
}

void ClientGUI::drawEnemies() {
    if (world_map.empty() || !enemy_texture || !tilemap) return;
    int tileSize = tilemap->getTileSize();
    for (int row = 0; row < static_cast<int>(world_map.size()); ++row) {
        for (int col = 0; col < static_cast<int>(world_map[row].size()); ++col) {
            if (world_map[row][col] == elements::npcs) {
                SDL_FRect dst = {
                    static_cast<float>(col * tileSize),
                    static_cast<float>(row * tileSize),
                    static_cast<float>(tileSize),
                    static_cast<float>(tileSize)
                };
                SDL_RenderTexture(renderer, enemy_texture, nullptr, &dst);
            }
        }
    }
}

void ClientGUI::drawAttackButton() {
    if (!show_attack_button) return;
    SDL_FRect btn = {10, 10, 120, 40};
    SDL_SetRenderDrawColor(renderer, 200, 50, 50, 255);
    SDL_RenderFillRect(renderer, &btn);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderRect(renderer, &btn);
}

void ClientGUI::drawInventoryPanel() {
    // Fondo del panel derecho
    SDL_FRect panel = {static_cast<float>(GAME_WIDTH), 0,
                       static_cast<float>(PANEL_WIDTH), static_cast<float>(WINDOW_HEIGHT)};
    SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255);
    SDL_RenderFillRect(renderer, &panel);
    SDL_SetRenderDrawColor(renderer, 180, 180, 180, 255);
    SDL_RenderRect(renderer, &panel);

    // Titulo "Inventario"
    if (chat_font) {
        SDL_Color white = {255, 255, 255, 255};
        SDL_Surface* surf = TTF_RenderText_Solid(chat_font, "Inventario", 0, white);
        if (surf) {
            SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surf);
            SDL_FRect dst = {static_cast<float>(GAME_WIDTH + 10), 10,
                             static_cast<float>(surf->w), static_cast<float>(surf->h)};
            SDL_RenderTexture(renderer, tex, nullptr, &dst);
            SDL_DestroyTexture(tex);
            SDL_DestroySurface(surf);
        }
    }

    // Items del inventario
    int y_offset = 40;
    for (const auto& item : inventory) {
        if (chat_font) {
            SDL_Color color = {220, 220, 220, 255};
            SDL_Surface* surf = TTF_RenderText_Solid(chat_font, item.get_name().c_str(), 0, color);
            if (surf) {
                SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surf);
                SDL_FRect dst = {static_cast<float>(GAME_WIDTH + 10), static_cast<float>(y_offset),
                                 static_cast<float>(surf->w), static_cast<float>(surf->h)};
                SDL_RenderTexture(renderer, tex, nullptr, &dst);
                SDL_DestroyTexture(tex);
                SDL_DestroySurface(surf);
            }
        }
        y_offset += 25;
    }

    // Boton "Pegar" en la parte inferior del panel
    if (show_attack_button) {
        SDL_FRect btn = {static_cast<float>(GAME_WIDTH + 10),
                         static_cast<float>(WINDOW_HEIGHT - 60),
                         static_cast<float>(PANEL_WIDTH - 20), 40};
        SDL_SetRenderDrawColor(renderer, 200, 50, 50, 255);
        SDL_RenderFillRect(renderer, &btn);
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderRect(renderer, &btn);

        if (chat_font) {
            SDL_Color white = {255, 255, 255, 255};
            SDL_Surface* surf = TTF_RenderText_Solid(chat_font, "PEGAR", 0, white);
            if (surf) {
                SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surf);
                SDL_FRect dst = {btn.x + (btn.w - surf->w) / 2.0f,
                                 btn.y + (btn.h - surf->h) / 2.0f,
                                 static_cast<float>(surf->w), static_cast<float>(surf->h)};
                SDL_RenderTexture(renderer, tex, nullptr, &dst);
                SDL_DestroyTexture(tex);
                SDL_DestroySurface(surf);
            }
        }
    }
}

void ClientGUI::draw() {
    SDL_RenderClear(renderer);
    if (tilemap) {
        tilemap->render();
    }
    drawEnemies();
    if (player) {
        player->draw();
    }
    drawInventoryPanel();
    mini_chat->render();
    SDL_RenderPresent(renderer);
}

void ClientGUI::init_draw() {
    // la info se la pasa el cliente desde config?
    initSDL();
    // aca recibe del protocolo la zona
    // hardocodeado para test
    loadMedia(zones::DESERT);

    SDL_Surface* enemy_surf = IMG_Load("imagenes/enemigo.png");
    if (!enemy_surf) enemy_surf = IMG_Load("images/enemigo.png");
    if (!enemy_surf) enemy_surf = IMG_Load("enemigo.png");
    if (enemy_surf) {
        enemy_texture = SDL_CreateTextureFromSurface(renderer, enemy_surf);
        SDL_DestroySurface(enemy_surf);
    }

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
