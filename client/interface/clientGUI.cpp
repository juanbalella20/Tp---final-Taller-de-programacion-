#include "clientGUI.h"
#include "npcSprite.h"
#include "itemSprite.h"
#include <SDL3_image/SDL_image.h>
#include <algorithm>
#include <iostream>
#include <stdexcept>

ClientGUI::ClientGUI(Queue<ClientCmd>& outgoing, Queue<GameMsg>& receiving, const std::string& player_name)
    : window(nullptr), renderer(nullptr), event{}, chat_font(nullptr),
      is_running(false), mini_chat(nullptr), parser(), outgoing(outgoing), receiving(receiving),
      hud(nullptr), own_name(player_name), player(nullptr), tilemap(nullptr),
      enemy_texture(nullptr), frame_texture(nullptr), item_texture(nullptr), gold_texture(nullptr),
      seller_texture(nullptr), banker_texture(nullptr), priest_texture(nullptr), camera((float)GAME_WIDTH, (float)CANVAS_HEIGHT),
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
void ClientGUI::loadMedia(Zone zone) {
    switch (zone)
    {
    case ZONE_DESERT : {
        tilemap = std::make_unique<TileMap>(renderer);
        tilemap->load_map("data/maps/desert/map.toml");
        break;
    }
    case ZONE_CITY : {
        tilemap = std::make_unique<TileMap>(renderer);
        tilemap->load_map("data/maps/city/map.toml");
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
    if (seller_texture) { 
        SDL_DestroyTexture(seller_texture); 
        seller_texture = nullptr; 
    }
    if (banker_texture) { 
        SDL_DestroyTexture(banker_texture); 
        banker_texture = nullptr; 
    }
    if (priest_texture) { 
        SDL_DestroyTexture(priest_texture); 
        priest_texture = nullptr;
    }
    if (frame_texture) {
        SDL_DestroyTexture(frame_texture);
        frame_texture = nullptr;
    }

    if (item_texture) {
        SDL_DestroyTexture(item_texture);
        item_texture = nullptr;
    }

    if (gold_texture) {
        SDL_DestroyTexture(gold_texture);
        gold_texture = nullptr;
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
/*
GameMap::MoveResult GameMap::try_move(Direction dir, const std::string& player_name) {
    Player* player = find_player_by_name(player_name);
    if (player == nullptr) {
        return {false, player_name, 0, 0};
    }

    int new_x = player->get_coord_x() + dir_to_dx(dir);
    int new_y = player->get_coord_y() + dir_to_dy(dir);

    std::cout << "[DEBUG: try_move " << player_name
              << "] (" << new_x << "," << new_y << ")" << std::endl;

    // Limites del mapa.
    if (new_x < 0 || new_y < 0 || new_x >= width || new_y >= height) {
        return {false, player_name, 0, 0};
    }
    // Terreno bloqueado (edificio).
    if (map[new_y][new_x] != elements::empty) {
    // VER
        return {false, player_name, 0, 0};
    }
    // Actor en la celda destino.
    if (has_actor_at(new_x, new_y)) {
        return {false, player_name, 0, 0};
    }
     for (const auto& gi : ground_items) {
        if (gi.pos.x == new_x && gi.pos.y == new_y) {
            return {false, player_name, 0, 0};
        }
    */
        
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
   //                     if (player) { player->setTilePosition(player->getTileX(), player->getTileY() - 1); player_pov = player->back_pov(); }
                        break;
                    case SDL_SCANCODE_DOWN:
                    case SDL_SCANCODE_S:
                        sendMoveCmd(DIR_SOUTH);
     //                   if (player) { player->setTilePosition(player->getTileX(), player->getTileY() + 1); player_pov = player->front_pov(); }
                        break;
                    case SDL_SCANCODE_RIGHT:
                    case SDL_SCANCODE_D:
                        sendMoveCmd(DIR_EAST);
       //                 if (player) { player->setTilePosition(player->getTileX() + 1, player->getTileY()); player_pov = player->right_pov(); }
                        break;
                    case SDL_SCANCODE_LEFT:
                    case SDL_SCANCODE_A:
                        sendMoveCmd(DIR_WEST);
         //               if (player) { player->setTilePosition(player->getTileX() - 1, player->getTileY()); player_pov = player->left_pov(); }
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
                        const auto& inv = hud->get_inventory();
                        int slot_index = 0;
                        for (const auto& item : inv) {
                            if (mx >= slot_x && mx <= slot_x + SLOT_SIZE &&
                                my >= slot_y && my <= slot_y + SLOT_SIZE) {
                                // Si el item ya está equipado, desquiparlo
                                // if (hud->get_equipped_slot() == slot_index) {
                                //     hud->set_equipped_slot(-1);
                                //     player->set_equipped_weapon(false);
                                // } else {
                                    // Equipar el nuevo item
                                hud->set_equipped_slot(slot_index);
                                player->set_equipped_weapon(true);
                                sendEquipCmd(item.get_id());
                                // }
                                break;
                            }
                            slot_x += SLOT_SIZE + SLOT_MARGIN;
                            ++slot_index;
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
        std::cout << "[DEBUG: sendChatCmd] selected_npc=(" 
                  << selected_npc_tile_x << "," << selected_npc_tile_y << ")" << std::endl;
        // Si es vender o listar, agregar coordenadas del NPC seleccionado
        if ((cmd.get_message_type() == MSG_SELL || 
             cmd.get_message_type() == MSG_BUY  ||
             cmd.get_message_type() == MSG_LIST)
            && selected_npc_tile_x != -1) {
            ClientCmd select_cmd;
            select_cmd.set_message_type(MSG_SELECT);
            select_cmd.set_coord_x(selected_npc_tile_x);
            select_cmd.set_coord_y(selected_npc_tile_y);
            outgoing.push(select_cmd);
        }
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
                case MSG_ZONE_CHANGE : {
                    Zone z = msg.get_zone();
                    if (z != current_zone) loadMedia(z);
                    player->setTilePosition(msg.get_coord_x(), msg.get_coord_y());
                    break;
                }
                case MSG_REGISTER: {
                    world_map = msg.get_map();
                    if (hud) {
                        hud->set_inventory(msg.get_items());
                        hud->set_gold(msg.get_gold());
                        hud->set_hp(msg.get_hp());
                        hud->set_xp(msg.get_xp());
                        hud->set_mana(msg.get_mana());
                    }
                    if (player) {//spawn
                        player->setTilePosition(msg.get_coord_x(), msg.get_coord_y());
                        std::cout << "Player registered at (" << msg.get_coord_x() << "," << msg.get_coord_y() << ")" << std::endl;
                    }
                    other_players = msg.get_players();
                    break;
                }
                case MSG_SEND_MAP:
                    world_map = msg.get_map();
                    break;
                case MSG_INVENTORY:
                    if (hud) hud->set_inventory(msg.get_items());
                    break;
                case MSG_NPCS_SNAPSHOT:
                    npcs = msg.get_npcs();
                    break;
                case MSG_ITEMS_SNAPSHOT:
                    items_on_floor = msg.get_items_on_floor();
                    break;
                case MSG_PLAYERS_SNAPSHOT:
                    std::cout << "Received players snapshot with " << msg.get_players().size() << " players." << std::endl;
                    for (const auto& incoming : msg.get_players()) {
                        auto it = std::find_if(other_players.begin(), other_players.end(),
                            [&incoming](const PlayerInfo& p) { return p.name == incoming.name; });
                        if (it != other_players.end()) {
                            *it = incoming;  // actualiza posicion si ya existe
                        } else {
                            other_players.push_back(incoming);  // nuevo jugador
                        }
                    }
                    break;
                case MSG_MOVE: {
                    const std::string& mover = msg.get_player_name();
                    int x = msg.get_coord_x();
                    int y = msg.get_coord_y();
                    int old_x = player->getTileX();
                    int old_y = player->getTileY();

                    
                    bool moved = (old_x != x || old_y != y);
                    if (!moved) player->reset_frame();
                    

                    // Nombre vacio = compatibilidad: tratar como jugador local.
                    if (mover.empty() || mover == own_name) {
                        // Jugador local: usar las coords absolutas del server.
                        player->setTilePosition(x, y);
                        switch (msg.get_direction()) {
                            case DIR_NORTH: player_pov = player->back_pov();  break;
                            case DIR_SOUTH: player_pov = player->front_pov(); break;
                            case DIR_EAST:  player_pov = player->right_pov(); break;
                            case DIR_WEST:  player_pov = player->left_pov();  break;
                            default: break;
                        }
                    } else {
                        // Otro jugador: actualizar (o agregar) su posicion.
                        auto it = std::find_if(other_players.begin(), other_players.end(),
                            [&mover](const PlayerInfo& p) { return p.name == mover; });
                        Direction dir = static_cast<Direction>(msg.get_direction());
                        if (it != other_players.end()) {
                            it->x = x;
                            it->y = y;
                            it->direction = dir;
                        } else {
                            PlayerInfo pi{mover, 0, 0, x, y, dir};
                            other_players.push_back(pi);
                        }
                    }
                    break;
                }
                case MSG_PRIVATE:
                case MSG_MEDITATE:
                case MSG_RESURRECT:
                case MSG_CURE:
                case MSG_LIST:
                case MSG_BUY:
                case MSG_SELL:
                case MSG_TAKE:
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
                case MSG_CHEAT_INF_HP:
                case MSG_CHEAT_INF_MANA:
                    chat_inbox.push(msg.get_chat_content());
                    break;
                case MSG_CHEAT_KILL:
                    std::cout << "DEBUG murió" << std::endl;
                    player->set_ghost(true);
                    chat_inbox.push(msg.get_chat_content());
                    break;
                case MSG_GOLD:
                    if (hud) hud->set_gold(msg.get_gold());
                    break;
                case MSG_HP:
                    if (hud) hud->set_hp(msg.get_hp());
                    break;
                case MSG_XP:
                    if (hud) hud->set_xp(msg.get_xp());
                    break;
                case MSG_MANA:
                    if (hud) hud->set_mana(msg.get_mana());
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
        NpcSprite(renderer, enemy_texture, npc.x, npc.y, tileSize).draw(camera, {});
    }
}

/*
void ClientGUI::draw_npc_friends() {
    if (!enemy_texture || !tilemap) return;
    const int tileSize = tilemap->getTileSize();

    for (size_t y = 0; y < world_map.size(); ++y) {
        const auto& row = world_map[y];
        for (size_t x = 0; x < row.size(); ++x) {
            if (row[x] != elements::npcs) continue;
            NpcSprite(renderer, enemy_texture,
                      static_cast<int>(x), static_cast<int>(y), tileSize)
                .draw(camera, {});
        }
    }
}*/

void ClientGUI::draw_npc_friends() {
    if (!tilemap) return;
    const int tileSize = tilemap->getTileSize();
    for (const auto& npc : npcs) {
        SDL_Texture* tex = nullptr;
        if (npc.type == "seller") tex = seller_texture;
        else if (npc.type == "banker") tex = banker_texture;
        else if (npc.type == "priest") tex = priest_texture;
        else continue;
        if (!tex) continue;
        SDL_FRect src = {0.0f, 0.0f, 30.0f, 40.0f};
        NpcSprite(renderer, tex, npc.x, npc.y, tileSize).draw(camera, src);
    }
}

void ClientGUI::drawOtherPlayers() {
    if (!tilemap) return;
    const int tileSize = tilemap->getTileSize();

    for (const auto& p : other_players) {
        try {
            PlayerDisplay pd(renderer, "imagenes/1005.png", tileSize);
            pd.setTilePosition(p.x, p.y);
            SDL_FRect pov;
            switch (p.direction) {
                case DIR_NORTH: pov = pd.back_pov();  break;
                case DIR_SOUTH: pov = pd.front_pov(); break;
                case DIR_EAST:  pov = pd.right_pov(); break;
                case DIR_WEST:  pov = pd.left_pov();  break;
                default:        pov = pd.front_pov(); break;
            }
            pd.draw(camera, pov);
        } catch (const std::runtime_error& e) {
            std::cout << "[DEBUG: drawOtherPlayers] textura fallida: " << e.what() << std::endl;
        }
    }
}
void ClientGUI::draw_teleport_labels() {
    if (!tilemap || !chat_font) return;
    const int tileSize = tilemap->getTileSize();
    const SDL_Color color = {255, 255, 0, 255};

    for (const auto& tp : tilemap->getTeleports()) {
        const std::string text = "Transportarse a " + tp.dest_zone;
        SDL_Surface* surf = TTF_RenderText_Blended(chat_font, text.c_str(), 0, color);
        if (!surf) continue;
        SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surf);
        if (tex) {
            const float wx = static_cast<float>(tp.x * tileSize);
            const float wy = static_cast<float>(tp.y * tileSize);
            const float sx = camera.world_to_screen_x(wx) + (tileSize - surf->w) / 2.0f;
            const float sy = camera.world_to_screen_y(wy) - surf->h;
            SDL_FRect dst{sx, sy, static_cast<float>(surf->w), static_cast<float>(surf->h)};
            SDL_RenderTexture(renderer, tex, nullptr, &dst);
            SDL_DestroyTexture(tex);
        }
        SDL_DestroySurface(surf);
    }
}

void ClientGUI::drawItems() {
    if (!tilemap) return;
    const int tileSize = tilemap->getTileSize();
    for (const auto& item : items_on_floor) {
        if (item.type == "gold") {
            if (!gold_texture) continue;
            SDL_FRect gold_cutout = { 0.0f, 320.0f, 30.0f, 27.0f };
            ItemSprite(renderer, gold_texture, item.x, item.y, tileSize).draw(camera, gold_cutout);
        } else {
            if (!item_texture) continue;
            SDL_FRect crop = {224.0f, 96.0f, 30.0f, 30.0f};
            ItemSprite(renderer, item_texture, item.x, item.y, tileSize).draw(camera, crop);
        }
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

    draw_teleport_labels();
    drawItems();
    if (player) {
        player->draw(camera, player_pov);
    }
    drawEnemies();
    drawOtherPlayers();
    draw_npc_friends();

    // Levanta el clip para dibujar el panel y el chat encima
    SDL_SetRenderClipRect(renderer, nullptr);

    if (hud) {
        hud->drawInventoryPanel();
        hud->drawAttackButton();
        hud->draw_hp();
        hud->draw_mana();
        hud->draw_gold();
        hud->draw_xp();
    }
    mini_chat->render(GAME_WIDTH, CANVAS_HEIGHT);
    SDL_RenderPresent(renderer);
}

void ClientGUI::init_draw() {
    initSDL();
    // se carga una zona inicial por default
    // siempre sera ZONA_CITY
    loadMedia(ZONE_CITY);
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

    SDL_Surface* item_surf = IMG_Load("imagenes/101.png");
    if (!item_surf) { 
        item_surf = IMG_Load("101.png");
    }
    if (item_surf) {
        item_texture = SDL_CreateTextureFromSurface(renderer, item_surf);
        SDL_SetTextureBlendMode(item_texture, SDL_BLENDMODE_BLEND);
        SDL_DestroySurface(item_surf);
    }

    SDL_Surface* elem_surf = IMG_Load("imagenes/100.png");
    if (!elem_surf) { elem_surf = IMG_Load("100.png"); }
    if (elem_surf) {
        gold_texture = SDL_CreateTextureFromSurface(renderer, elem_surf);
        SDL_DestroySurface(elem_surf);
    }

    SDL_Surface* seller_surf = IMG_Load("imagenes/4055.png");
    if (seller_surf) {
        seller_texture = SDL_CreateTextureFromSurface(renderer, seller_surf);
        SDL_DestroySurface(seller_surf);
    }
    SDL_Surface* banker_surf = IMG_Load("imagenes/4051.png");
    if (banker_surf) {
        banker_texture = SDL_CreateTextureFromSurface(renderer, banker_surf);
        SDL_DestroySurface(banker_surf);
    }
    SDL_Surface* priest_surf = IMG_Load("imagenes/4057.png");
    if (priest_surf) {
        priest_texture = SDL_CreateTextureFromSurface(renderer, priest_surf);
        SDL_DestroySurface(priest_surf);
    }

    hud = std::make_unique<HUD>(renderer, GAME_WIDTH, PANEL_WIDTH, CANVAS_HEIGHT);

    // tile_size viene del TOML
    // el player se escala con el mismo tamano de celda
    int tileSize = tilemap->getTileSize();
    try {
        player = std::make_unique<PlayerDisplay>(renderer, "imagenes/1005.png", tileSize);
        player_pov = player->back_pov();
    } catch (const std::runtime_error& e) {
        std::cout << "[DEBUG] imagenes/1005.png failed: " << e.what() << std::endl;
    }
    // La posicion real llega en MSG_REGISTER via update().
    // Posicion temporal (0,0) hasta que llegue el mensaje del servidor.
    player->setTilePosition(0, 0);
    draw_npc_friends();

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
