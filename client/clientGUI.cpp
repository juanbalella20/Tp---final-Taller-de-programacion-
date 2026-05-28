#include "clientGUI.h"
#include "npcSprite.h"
#include <SDL3_image/SDL_image.h>
#include <iostream>
#include <stdexcept>

ClientGUI::ClientGUI(Queue<ClientCmd>& outgoing, Queue<GameMsg>& receiving)
    : window(nullptr), renderer(nullptr), event{}, chat_font(nullptr),
      is_running(false), mini_chat(nullptr), parser(), outgoing(outgoing), receiving(receiving),
      hud(nullptr),
      enemy_texture(nullptr), frame_texture(nullptr),
      camera((float)GAME_WIDTH, (float)CANVAS_HEIGHT),
      selected_npc_tile_x(-1), selected_npc_tile_y(-1) {}
    

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

    SDL_Surface* icon = IMG_Load("imagenes/logo.jpeg");
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
    hud.reset();

    if (enemy_texture) {
        SDL_DestroyTexture(enemy_texture);
        enemy_texture = nullptr;
    }
    if (frame_texture) {
        SDL_DestroyTexture(frame_texture);
        frame_texture = nullptr;
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
    int world_x = static_cast<int>(camera.screen_to_world_x(pixel_x));
    int world_y = static_cast<int>(camera.screen_to_world_y(pixel_y));
    return {world_x / tileSize, world_y / tileSize};
}

void ClientGUI::sendAttackCmd(int tile_x, int tile_y) {
    ClientCmd cmd;
    cmd.set_message_type(MSG_ATTACK);
    cmd.set_coord_x(tile_x);
    cmd.set_coord_y(tile_y);
    outgoing.push(cmd);
}

void ClientGUI::sendEquipCmd(const std::string& item_id) {
    ClientCmd cmd;
    cmd.set_message_type(MSG_EQUIP);
    cmd.set_item_id(item_id);
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
    bool npc_clicked = false;
    for (const auto& npc : npcs) {
        if (npc.x == tile_x && npc.y == tile_y) {
            npc_clicked = true;
            break;
        }
    }

    if (npc_clicked) {
        selected_npc_tile_x = tile_x;
        selected_npc_tile_y = tile_y;
        if (hud) hud->set_attack_button_visible(true);
    } else {
        if (hud) hud->set_attack_button_visible(false);
        selected_npc_tile_x = -1; // revisar
        selected_npc_tile_y = -1; // revisar
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
                float lx, ly;
                SDL_RenderCoordinatesFromWindow(renderer, event.button.x, event.button.y, &lx, &ly);
                int mx = static_cast<int>(lx);
                int my = static_cast<int>(ly);

                if (mx >= GAME_WIDTH) {
                    // Click dentro del panel — nunca toca el mapa
                    if (hud && hud->is_attack_button_visible()) {
                        const SDL_FRect& btn = hud->get_attack_button_rect();
                        if (mx >= btn.x && mx <= btn.x + btn.w &&
                            my >= btn.y && my <= btn.y + btn.h) {
                            sendAttackCmd(selected_npc_tile_x, selected_npc_tile_y);
                            hud->set_attack_button_visible(false);
                            break;
                        }
                    }
                    const int SLOT_SIZE = 48;
                    const int SLOT_MARGIN = 8;
                    int slot_x = GAME_WIDTH + SLOT_MARGIN;
                    int slot_y = 40;
                    if (hud) {
                        for (const auto& item : hud->get_inventory()) {
                            if (mx >= slot_x && mx <= slot_x + SLOT_SIZE &&
                                my >= slot_y && my <= slot_y + SLOT_SIZE) {
                                sendEquipCmd(item.get_id());
                                break;
                            }
                            slot_x += SLOT_SIZE + SLOT_MARGIN;
                            if (slot_x + SLOT_SIZE > GAME_WIDTH + PANEL_WIDTH - SLOT_MARGIN) {
                                slot_x = GAME_WIDTH + SLOT_MARGIN;
                                slot_y += SLOT_SIZE + SLOT_MARGIN;
                            }
                        }
                    }
                } else {
                    // Click en el area del juego
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
                    if (hud) hud->set_inventory(msg.get_items());
                    break;
                case MSG_NPCS_SNAPSHOT:
                    npcs = msg.get_npcs();
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
    if (!enemy_texture || !tilemap) return;
    const int tileSize = tilemap->getTileSize();
    for (const auto& npc : npcs) {
        NpcSprite(renderer, enemy_texture, npc.x, npc.y, tileSize).draw(camera);
    }
}

#define TILESIZE 64
void ClientGUI::draw() {
    // centrar camara en el jugador (en el centro del tile) y limitar al mapa
    camera.center_on(player->get_x() + TILESIZE / 2.0f,
                     player->get_y() + TILESIZE / 2.0f);
    if (tilemap) {
        camera.clamp_to(static_cast<float>(tilemap->getPixelWidth()),
                        static_cast<float>(tilemap->getPixelHeight()));
    }

    SDL_RenderClear(renderer);

    // Limita el rendering del mapa y entidades al area del juego (excluye panel derecho)
    SDL_Rect game_clip = {0, 0, GAME_WIDTH, CANVAS_HEIGHT};
    SDL_SetRenderClipRect(renderer, &game_clip);

    if (tilemap) {
        tilemap->render(camera.get_x(), camera.get_y());
    }
    if (player) {
        player->draw(camera);
    }
    drawEnemies();
    

    // Levanta el clip para dibujar el panel y el chat encima
    SDL_SetRenderClipRect(renderer, nullptr);

    if (hud) {
        hud->drawInventoryPanel();
        hud->drawAttackButton();
        /* TODO:
        hud->draw_hp();
        hud->draw_mana();
        hud->draw_gold();
        hud->draw_xp();
        */
    }
    mini_chat->render(GAME_WIDTH, CANVAS_HEIGHT);
    SDL_RenderPresent(renderer);
}

void ClientGUI::init_draw() {
    // la info se la pasa el cliente desde config?
    initSDL();
    // aca recibe del protocolo la zona
    // hardocodeado para test
    loadMedia(zones::DESERT);

    SDL_Surface* enemy_surf = IMG_Load("imagenes/enemigo.png");
    if (!enemy_surf) enemy_surf = IMG_Load("enemigo.png");
    if (enemy_surf) {
        enemy_texture = SDL_CreateTextureFromSurface(renderer, enemy_surf);
        SDL_DestroySurface(enemy_surf);
    }

    SDL_Surface* frame_surf = IMG_Load("imagenes/frame..png");
    if (frame_surf) {
        frame_texture = SDL_CreateTextureFromSurface(renderer, frame_surf);
        SDL_DestroySurface(frame_surf);
    }
    hud = std::make_unique<HUD>(renderer, GAME_WIDTH, PANEL_WIDTH, CANVAS_HEIGHT);

    // tile_size viene del TOML
    // el player se escala con el mismo tamano de celda
    int tileSize = tilemap->getTileSize();
    try {
        player = std::make_unique<PlayerDisplay>(renderer, "imagenes/player.png", tileSize);
    } catch (const std::runtime_error& e) {
        std::cout << "[DEBUG] imagenes/player.png failed: " << e.what() << std::endl;
    }
    // valor hardcodeado para testing!
    // revisar game_map.cpp
    player->setTilePosition(29, 15);


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
