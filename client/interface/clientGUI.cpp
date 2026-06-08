#include "clientGUI.h"
#include "npcSprite.h"
#include "itemSprite.h"
#include <SDL3_image/SDL_image.h>
#include <algorithm>
#include <cmath>
#include <iostream>
#include <stdexcept>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

ClientGUI::ClientGUI(SDL_Window* window, SDL_Renderer* renderer, TTF_Font* font,
    Queue<ClientCmd>& outgoing, Queue<GameMsg>& receiving, const std::string& player_name,
    const std::string& player_race)
    : window(window), renderer(renderer), event{}, chat_font(font),
      is_running(false), mini_chat(nullptr), parser(), outgoing(outgoing), receiving(receiving),
      hud(nullptr), own_name(player_name), race(player_race), player(nullptr), tilemap(nullptr),
      enemy_texture(nullptr), frame_texture(nullptr), item_texture(nullptr), gold_texture(nullptr),
      camera((float)GAME_VIEW_W, (float)GAME_VIEW_H),
      current_zone(static_cast<Zone>(0xFF)),
      selected_npc_tile_x(-1), selected_npc_tile_y(-1) {}


ClientGUI::~ClientGUI() {
    freeSDL();
    // chat_font, window y renderer son del ScreenManager: no se liberan aca.
}

void ClientGUI::initSDL() {
    // window/renderer/font ya existen (del ScreenManager). Solo configuramos la
    // presentacion logica del juego, el icono y el mini chat.
    SDL_SetRenderLogicalPresentation(renderer, LOGICAL_WIDTH, LOGICAL_HEIGHT, SDL_LOGICAL_PRESENTATION_LETTERBOX);

    SDL_Surface* icon = IMG_Load("imagenes/logo.jpeg");
    if (icon) {
        SDL_SetWindowIcon(window, icon);
        SDL_DestroySurface(icon);
    }

    mini_chat = std::make_unique<MiniChat>(renderer, chat_font, GAME_WIDTH, PANEL_WIDTH, CANVAS_HEIGHT);
}

// tiene que recibir los 4 sectorPerimiter y mostrar solo eso
void ClientGUI::loadMedia(Zone zone) {
    switch (zone)
    {
    case ZONE_DESERT : {
        tilemap = std::make_unique<TileMap>(renderer);
        tilemap->load_map_bin("data/maps/desert/map-2.bin");
        break;
    }
    case ZONE_CITY : {
        tilemap = std::make_unique<TileMap>(renderer);
        tilemap->load_map_bin("data/maps/city/city-2.bin");
        break;
    }
    case ZONE_FOREST : {
        tilemap = std::make_unique<TileMap>(renderer);
        tilemap->load_map_bin("data/maps/forest/forest2.bin");
        break;
    }
    case ZONE_DUNGEON : {
        tilemap = std::make_unique<TileMap>(renderer);
        tilemap->load_map_bin("data/maps/dungeon/dungeon-v1.bin");
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

    // Texturas extra de items del piso (las que no son item_texture, p. ej. escudo).
    // Se saltea gold_texture: lo comparten las pociones del piso y se libera aparte.
    for (auto& kv : floor_item_textures) {
        if (kv.second && kv.second != item_texture && kv.second != gold_texture) {
            SDL_DestroyTexture(kv.second);
        }
    }
    floor_item_textures.clear();

    if (item_texture) {
        SDL_DestroyTexture(item_texture);
        item_texture = nullptr;
    }

    if (gold_texture) {
        SDL_DestroyTexture(gold_texture);
        gold_texture = nullptr;
    }

    // window/renderer NO se destruyen: son del ScreenManager. Tampoco SDL_Quit.
}

std::vector<int> ClientGUI::translate_tile_to_coord(int pixel_x, int pixel_y) const {
    int tileSize = tilemap ? tilemap->getTileSize() : 64;
    // El mundo se dibuja dentro del viewport del hueco, asi que hay que restar
    // su origen para pasar de coordenadas logicas a coordenadas del viewport.
    int world_x = static_cast<int>(camera.screen_to_world_x(pixel_x - GAME_VIEW_X));
    int world_y = static_cast<int>(camera.screen_to_world_y(pixel_y - GAME_VIEW_Y));
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

void ClientGUI::sendUseItemCmd(const std::string& item_uid) {
    ClientCmd cmd;
    cmd.set_message_type(MSG_USE_ITEM);
    cmd.set_item_id(item_uid);  // uid de INSTANCIA en texto (como MSG_EQUIP)
    outgoing.push(cmd);
}

bool ClientGUI::potion_slot_at(int mx, int my, uint64_t& out_uid,
                               SDL_FRect& out_rect) const {
    if (!hud) return false;
    const auto& inv = hud->get_inventory();

    // Misma geometría que el handler de equipar (click izquierdo) más abajo.
    float image_w = 1021.0f;
    float image_h = 767.0f;
    float scale_x = static_cast<float>(GAME_WIDTH + PANEL_WIDTH) / image_w;
    float scale_y = static_cast<float>(CANVAS_HEIGHT) / image_h;

    float slot_start_size = 48.0f;
    float slot_margin = 4.0f;
    float padding = 6.0f;
    float inv_x = 781.0f;
    float inv_y = 200.0f;
    float inv_w = 218.0f;

    float start_x = (inv_x + padding) * scale_x;
    float start_y = (inv_y + padding) * scale_y;
    float slot_size = slot_start_size * scale_x;
    float margin_x = slot_margin * scale_x;
    float margin_y = slot_margin * scale_y;
    float limit_x = (inv_x + inv_w - padding) * scale_x;

    float slot_x = start_x;
    float slot_y = start_y;
    for (const auto& item : inv) {
        if (mx >= slot_x && mx <= slot_x + slot_size &&
            my >= slot_y && my <= slot_y + slot_size) {
            // Solo las pociones (item consumible, ItemType::OTHER == 4) se toman
            // manteniendo presionado. Las armas/defensas se equipan con un click.
            if (item.get_type() != 4) return false;
            out_uid = item.get_uid();
            out_rect = {slot_x, slot_y, slot_size, slot_size};
            return true;
        }
        slot_x += slot_size + margin_x;
        if (slot_x + slot_size > limit_x) {
            slot_x = start_x;
            slot_y += slot_size + margin_y;
        }
    }
    return false;
}

void ClientGUI::update_potion_hold() {
    if (!potion_hold_active || potion_hold_sent) return;
    uint64_t elapsed = SDL_GetTicks() - potion_hold_start_ms;
    if (elapsed >= static_cast<uint64_t>(POTION_HOLD_MS)) {
        // Se completaron los 3s: pedir al server consumir la poción. El server
        // valida, aplica el efecto y reenvía el inventario (la poción desaparece).
        sendUseItemCmd(std::to_string(potion_hold_uid));
        potion_hold_sent = true;  // no repetir hasta soltar y volver a presionar
    }
}

void ClientGUI::draw_potion_hold_arc() {
    if (!potion_hold_active || potion_hold_sent) return;
    uint64_t elapsed = SDL_GetTicks() - potion_hold_start_ms;
    float progress = static_cast<float>(elapsed) / POTION_HOLD_MS;
    if (progress > 1.0f) progress = 1.0f;

    // Anillo de progreso centrado en el slot. Se dibuja como una serie de
    // segmentos a lo largo de un arco que va de 0 a (progress * 360°), arrancando
    // arriba (-90°) y avanzando en sentido horario.
    float cx = potion_hold_slot_rect.x + potion_hold_slot_rect.w / 2.0f;
    float cy = potion_hold_slot_rect.y + potion_hold_slot_rect.h / 2.0f;
    float radius = (potion_hold_slot_rect.w / 2.0f) + 4.0f;

    const int total_segments = 48;
    int lit = static_cast<int>(progress * total_segments);

    // Fondo del anillo (tenue) + progreso (brillante), dibujados como puntos
    // gruesos a lo largo de la circunferencia.
    for (int i = 0; i < total_segments; ++i) {
        float frac = static_cast<float>(i) / total_segments;
        float angle = -static_cast<float>(M_PI) / 2.0f + frac * 2.0f * static_cast<float>(M_PI);
        float px = cx + radius * std::cos(angle);
        float py = cy + radius * std::sin(angle);
        if (i < lit) {
            SDL_SetRenderDrawColor(renderer, 80, 220, 120, 255);  // verde brillante
        } else {
            SDL_SetRenderDrawColor(renderer, 40, 60, 40, 120);    // fondo tenue
        }
        // "Punto" de 3x3 para que el anillo se vea grueso.
        SDL_FRect dot = {px - 1.5f, py - 1.5f, 3.0f, 3.0f};
        SDL_RenderFillRect(renderer, &dot);
    }
}

void ClientGUI::sendCoord(int tile_x, int tile_y) {
    ClientCmd cmd;
    cmd.set_message_type(MSG_SELECT);
    cmd.set_coord_x(tile_x);
    cmd.set_coord_y(tile_y);
    outgoing.push(cmd);
}

void ClientGUI::selectCoord(int tile_x, int tile_y) {
  bool position_other_player = false;
  bool position_npc = false;
    // Click sobre la propia celda: lanzar el hechizo del báculo equipado sobre
    // sí mismo (auto-cast, p.ej. curación). El server decide si aplica.
    if (player && player->getTileX() == tile_x && player->getTileY() == tile_y) {
        sendSelfCastCmd();
        return;
    }
    for (const auto& p : other_players) {
        if (p.x == tile_x && p.y == tile_y) {
            position_other_player = true;
            break;
        }
    }
    for (const auto& npc : npcs) {
        if (npc.x == tile_x && npc.y == tile_y) {
            position_npc = true;
            break;
        }
    }
    if (position_other_player || position_npc) {
        sendAttackCmd(tile_x, tile_y);
    } else {
        sendCoord(tile_x, tile_y);
    }
}

void ClientGUI::sendSelfCastCmd() {
    ClientCmd cmd;
    cmd.set_message_type(MSG_SELF_CAST);
    outgoing.push(cmd);
    // Efecto de curación sobre el propio sprite. Solo si hay un báculo de
    // curación (flauta élfica) equipado; el server valida maná/clase aparte.
    if (player && equipped_spell_id == "flauta_elfica") {
        spawn_spell_animation(equipped_spell_id, player->getTileX(), player->getTileY());
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

                if (event.button.button != SDL_BUTTON_LEFT) break;

                // Click IZQUIERDO sostenido sobre una poción: arranca el hold de 3s
                // para tomarla. El conteo y el envío del use los maneja
                // update_potion_hold(). Una poción no se equipa, así que el hold
                // reemplaza al equip sólo en sus slots; el resto sigue equipando.
                {
                    uint64_t uid = 0;
                    SDL_FRect slot_rect;
                    if (potion_slot_at(mx, my, uid, slot_rect)) {
                        potion_hold_active = true;
                        potion_hold_sent = false;
                        potion_hold_uid = uid;
                        potion_hold_start_ms = SDL_GetTicks();
                        potion_hold_slot_rect = slot_rect;
                        break;  // sobre una poción: hold, no equipar
                    }
                }

                if (mx >= GAME_WIDTH) {
                    if (hud) {
                        const auto& inv = hud->get_inventory();
                        float image_w = 1021.0f;
                        float image_h = 767.0f;

                        float scale_x = static_cast<float>(GAME_WIDTH + PANEL_WIDTH) / image_w;
                        float scale_y = static_cast<float>(CANVAS_HEIGHT) / image_h;

                        float slot_start_size = 48.0f;
                        float slot_margin = 4.0f;
                        float padding = 6.0f;

                        float inv_x = 781.0f;
                        float inv_y = 200.0f;
                        float inv_w = 218.0f;
                        float start_x = (inv_x + padding) * scale_x;
                        float start_y = (inv_y + padding) * scale_y;

                        float slot_size = slot_start_size * scale_x;
                        float margin_x = slot_margin * scale_x;
                        float margin_y = slot_margin * scale_y;

                        float limit_x = (inv_x + inv_w - padding) * scale_x;

                        float slot_x = start_x;
                        float slot_y = start_y;
                        int slot_index = 0;
                        for (const auto& item : inv) {
                            if (mx >= slot_x && mx <= slot_x + slot_size &&
                                my >= slot_y && my <= slot_y + slot_size) {
                                std::cout << "DEBUG mx: " << mx << " my: " << my << std::endl;
                                // Sólo pedimos el toggle al server. Él responde con
                                // MSG_UPDATE_EQUIP y de ahí derivamos el halo amarillo
                                // y el arma del personaje (estado real, no optimista).
                                // Enviamos el uid de INSTANCIA (no el id de tipo): así
                                // con dos copias del mismo item se equipa exactamente
                                // la que se clickeó.
                                sendEquipCmd(std::to_string(item.get_uid()));
                                break;
                            }
                            slot_x += slot_size + margin_x;
                            if (slot_x + slot_size > limit_x) {
                                slot_x = start_x;
                                slot_y += slot_size + margin_y;
                            }
                            ++slot_index;
                        }
                    }
                } else if (mx >= GAME_VIEW_X && mx < GAME_VIEW_X + GAME_VIEW_W &&
                           my >= GAME_VIEW_Y && my < GAME_VIEW_Y + GAME_VIEW_H) {
                    // Click dentro del hueco visible del mundo (no en el chat ni los bordes).
                    auto coords = translate_tile_to_coord(mx, my);
                    selectCoord(coords[0], coords[1]);
                }
                break;
            }
            case SDL_EVENT_MOUSE_BUTTON_UP: {
                // Soltar el botón izquierdo antes de los 3s cancela el hold: la
                // poción no se toma (hay que volver a mantener presionado).
                if (event.button.button == SDL_BUTTON_LEFT) {
                    potion_hold_active = false;
                    potion_hold_sent = false;
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
        if (!player) {
            return;
        }
        GameMsg msg(0);
        while (receiving.try_pop(msg)) {
            switch (msg.get_type()) {
                case MSG_ZONE_CHANGE : {
                    Zone z = msg.get_zone();
                    if (z != current_zone) {
                        loadMedia(z);
                        current_zone = z;
                    }
                    player->setTilePosition(msg.get_coord_x(), msg.get_coord_y());
                    break;
                }
                case MSG_REGISTER: {
                    world_map = msg.get_map();
                    if (hud) {
                        hud->set_inventory(msg.get_items());
                        hud->set_gold(msg.get_gold());
                        hud->set_hp(msg.get_hp());
                        hud->set_max_hp(msg.get_hp());
                        hud->set_max_mana(msg.get_mana());
                        hud->set_xp(msg.get_xp());
                        hud->set_max_xp(msg.get_max_xp());
                        hud->set_mana(msg.get_mana());
                        hud->set_level(msg.get_level());
                    }
                    if (player) {//spawn
                        player->setTilePosition(msg.get_coord_x(), msg.get_coord_y());
                        player->set_ghost(msg.get_ghost());
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
                case MSG_PLAYERS_SNAPSHOT: {
                    // std::cout << "Received players snapshot with " << msg.get_players().size() << " players." << std::endl;  // hot path: floodea la consola cada frame
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
                }
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
                            case DIR_NORTH: 
                                if (player->is_ghost()) {
                                    player_pov = player->ghost_frame();
                                } else {
                                    player_pov = player->back_pov(ViewDirection::BACK);
                                }
                                break;
                            case DIR_SOUTH:
                                if (player->is_ghost()) {
                                    player_pov = player->ghost_frame();
                                } else {
                                    player_pov = player->front_pov(ViewDirection::FRONT);
                                } 
                                break;
                            case DIR_EAST:
                                if (player->is_ghost()) {
                                    player_pov = player->ghost_frame();
                                } else {
                                    player_pov = player->right_pov(ViewDirection::RIGHT);
                                }  
                                break;
                            case DIR_WEST:
                                if (player->is_ghost()) {
                                    player_pov = player->ghost_frame();
                                } else {
                                    player_pov = player->left_pov(ViewDirection::LEFT);
                                }   
                                break;
                            default: break;
                        }
                    } else {
                        // Otro jugador: actualizar (o agregar) su posicion.
                        auto it = std::find_if(other_players.begin(), other_players.end(),
                            [&mover](const PlayerInfo& p) { return p.name == mover; });
                        Direction dir = static_cast<Direction>(msg.get_direction());
                        if (it != other_players.end()) {
                            bool ghost = it->ghost;
                            it->x = x;
                            it->y = y;
                            it->direction = dir;
                            it->ghost = ghost;
                        } else {
                            PlayerInfo pi{mover, msg.get_race(), 0, x, y, dir};
                            pi.ghost = false;
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
                case MSG_CHEAT_KILL: {
                    if (msg.get_player_name() == own_name) {
                        player->set_ghost(true);
                        player_pov = player->ghost_frame();
                    } else {
                        for (auto& p : other_players) {
                            if (p.name == msg.get_player_name()) {
                                p.ghost = true;
                                break;
                            }
                        }
                    }
                    chat_inbox.push(msg.get_chat_content());
                    break;
                }
                case MSG_GOLD:
                    if (hud) hud->set_gold(msg.get_gold());
                    break;
                case MSG_HP:
                    if (hud) hud->set_hp(msg.get_hp());
                    break;
                case MSG_ATTACK:
                    if (msg.get_damage() > 0) {
                        damage_numbers.push_back({
                            msg.get_coord_x(), msg.get_coord_y(),
                            msg.get_damage(), SDL_GetTicks() + DMG_MS
                        });
                    }
                    // Si el atacante tiene un báculo de daño equipado, anima su
                    // efecto sobre el target. La flauta (curación) no aplica acá:
                    // su efecto se dispara localmente al hacer /self-cast.
                    if (!equipped_spell_id.empty() && equipped_spell_id != "flauta_elfica") {
                        spawn_spell_animation(equipped_spell_id,
                                              msg.get_coord_x(), msg.get_coord_y());
                    }
                    break;
                case MSG_XP:
                    if (hud) hud->set_xp(msg.get_xp());
                    break;
                case MSG_MANA:
                    if (hud) hud->set_mana(msg.get_mana());
                    break;
                case MSG_UPDATE_EQUIP:
                    // El server confirma el estado real del equipo.
                    if (msg.get_player_name() == own_name) {
                        // Arma del personaje + halos del inventario (todos los items
                        // equipados, arma y defensas) según el estado real del server.
                        if (player) {
                            player->set_equipped_weapon(msg.get_equipped());
                            // Sprite del personaje con cada item equipado (arma/báculo + escudo).
                            player->set_equipped_items(msg.get_equipped_ids());
                        }
                        if (hud) hud->set_equipped_by_uids(msg.get_equipped_uids());
                        // ¿Equipó un báculo? Lo recordamos para animar su efecto.
                        equipped_spell_id.clear();
                        for (const auto& id : msg.get_equipped_ids()) {
                            if (spell_effects.find(id) != spell_effects.end()) {
                                equipped_spell_id = id;
                                break;
                            }
                        }
                    } else {
                        for (auto& p : other_players) {
                            if (p.name == msg.get_player_name()) {
                                p.has_equipped_weapon = msg.get_equipped();
                                break;
                            }
                        }
                    }
                    break;
                case MSG_UPDATE_LEVEL: {
                    if (hud) {
                        hud->set_level(msg.get_level());
                        hud->set_max_xp(msg.get_max_xp());
                    }
                    break;
                }
                case MSG_DEATH: {
                    if (msg.get_player_name() == own_name) {
                        player->set_ghost(true);
                        player_pov = player->ghost_frame();
                    } else {
                        for (auto& p : other_players) {
                            if (p.name == msg.get_player_name()) {
                                p.ghost = true;
                                break;
                            }
                        }
                    }
                    break;
                }

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
        NpcSprite(renderer, enemies_textures[npc.name], npc.x, npc.y, tileSize)
            .draw(camera, enemies_crops[npc.name]);
    }
}

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
}

void ClientGUI::drawOtherPlayers() {
    if (!tilemap) return;
    const int tileSize = tilemap->getTileSize();
    for (const auto& p : other_players) {
        try {
            PlayerDisplay pd(renderer, "imagenes/1005.png", tileSize, p.race);
            pd.setTilePosition(p.x, p.y);
            pd.set_equipped_weapon(p.has_equipped_weapon);
            pd.set_ghost(p.ghost);
            SDL_FRect pov;
            switch (p.direction) {
                case DIR_NORTH: 
                    if (pd.is_ghost()) {
                        pov = pd.ghost_frame();
                    } else {
                        pov = pd.back_pov(ViewDirection::BACK);
                    }
                    break;
                case DIR_SOUTH:
                    if (pd.is_ghost()) {
                        pov = pd.ghost_frame();
                    } else {
                        pov = pd.front_pov(ViewDirection::FRONT);
                    } 
                    break;
                case DIR_EAST:
                    if (pd.is_ghost()) {
                        pov = pd.ghost_frame();
                    } else {
                        pov = pd.right_pov(ViewDirection::RIGHT);
                    }  
                    break;
                case DIR_WEST:
                    if (pd.is_ghost()) {
                        pov = pd.ghost_frame();
                    } else {
                        pov = pd.left_pov(ViewDirection::LEFT);
                    }   
                    break;
                default: break;
            }
            pd.draw(camera, pov);
        } catch (const std::runtime_error& e) {
            std::cout << "[DEBUG: drawOtherPlayers] textura fallida: " << e.what() << std::endl;
        }
    }
}


void ClientGUI::load_spell_effects() {
    // Cada báculo tiene un spritesheet de efecto. La clave es el id del item
    // (igual que en el server) para deducir el efecto desde el item equipado.
    struct Def { const char* id; const char* path; int fw; int fh; int cols; int count; };
    const Def defs[] = {
        // Curación (flauta élfica): 1024x1024, grilla 5x2 -> 10 frames de 204x204.
        {"flauta_elfica",    "imagenes/3444.png", 204, 204, 5, 10},
        // Flecha mágica (vara de fresno): 512x512, grilla 4x4 -> 15 frames de 128x128.
        {"vara_fresno",      "imagenes/2511.png", 128, 128, 4, 15},
        // Misil (báculo nudoso): 512x512, grilla 4x4 -> 16 frames de 128x128.
        {"baculo_nudoso",    "imagenes/3451.png", 128, 128, 4, 16},
        // Explosión (báculo engarzado): 512x512, grilla 4x4 -> 16 frames de 128x128.
        {"baculo_engarzado", "imagenes/864.png",  128, 128, 4, 16},
    };
    for (const auto& d : defs) {
        SDL_Surface* surf = IMG_Load(d.path);
        if (!surf) continue;
        SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surf);
        SDL_DestroySurface(surf);
        if (!tex) continue;
        spell_effects[d.id] = {tex, d.fw, d.fh, d.cols, d.count};
    }
}

void ClientGUI::spawn_spell_animation(const std::string& effect_id, int tile_x, int tile_y) {
    if (spell_effects.find(effect_id) == spell_effects.end()) return;
    spell_animations.push_back({tile_x, tile_y, effect_id, SDL_GetTicks()});
}

void ClientGUI::draw_spell_animations() {
    if (!tilemap) return;
    const int tileSize = tilemap->getTileSize();
    const uint64_t now = SDL_GetTicks();
    constexpr uint64_t FRAME_MS = 55;  // duración de cada frame de la animación

    // Descarta las animaciones que ya terminaron todos sus frames.
    spell_animations.erase(
        std::remove_if(spell_animations.begin(), spell_animations.end(),
            [&](const SpellAnimation& a) {
                auto it = spell_effects.find(a.effect_id);
                if (it == spell_effects.end()) return true;
                uint64_t frame = (now - a.start_ms) / FRAME_MS;
                return frame >= static_cast<uint64_t>(it->second.frame_count);
            }),
        spell_animations.end());

    for (const auto& a : spell_animations) {
        const SpellEffectDef& def = spell_effects[a.effect_id];
        int frame = static_cast<int>((now - a.start_ms) / FRAME_MS);
        if (frame >= def.frame_count) continue;
        SDL_FRect src{
            static_cast<float>((frame % def.cols) * def.frame_w),
            static_cast<float>((frame / def.cols) * def.frame_h),
            static_cast<float>(def.frame_w),
            static_cast<float>(def.frame_h)
        };
        // Centrado y escalado para cubrir el tile del target (un poco más grande
        // que el tile para que el efecto envuelva al sprite).
        const float size = tileSize * 1.6f;
        const float wx = static_cast<float>(a.tile_x * tileSize);
        const float wy = static_cast<float>(a.tile_y * tileSize);
        const float sx = camera.world_to_screen_x(wx) + (tileSize - size) / 2.0f;
        const float sy = camera.world_to_screen_y(wy) + (tileSize - size) / 2.0f;
        SDL_FRect dst{sx, sy, size, size};
        SDL_RenderTexture(renderer, def.texture, &src, &dst);
    }
}

void ClientGUI::draw_damage_numbers() {
    if (!tilemap || !chat_font) return;
    const int tileSize = tilemap->getTileSize();
    const SDL_Color color = {255, 0, 0, 255};  // rojo
    const uint64_t now = SDL_GetTicks();

    // Descarta los numeros que ya expiraron.
    damage_numbers.erase(
        std::remove_if(damage_numbers.begin(), damage_numbers.end(),
                       [now](const DamageNumber& d) { return now >= d.expire_ms; }),
        damage_numbers.end());

    for (const auto& d : damage_numbers) {
        const std::string text = std::to_string(d.value);
        SDL_Surface* surf = TTF_RenderText_Blended(chat_font, text.c_str(), 0, color);
        if (!surf) continue;
        SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surf);
        if (tex) {
            const float wx = static_cast<float>(d.tile_x * tileSize);
            const float wy = static_cast<float>(d.tile_y * tileSize);
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
            // Textura y crop específicos del item por id; fallback a la espada.
            SDL_Texture* tex = item_texture;
            SDL_FRect crop = {224.0f, 96.0f, 30.0f, 30.0f};
            auto tex_it = floor_item_textures.find(item.type);
            auto crop_it = floor_item_crops.find(item.type);
            if (tex_it != floor_item_textures.end()) tex = tex_it->second;
            if (crop_it != floor_item_crops.end()) crop = crop_it->second;
            if (!tex) continue;
            ItemSprite(renderer, tex, item.x, item.y, tileSize).draw(camera, crop);
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

    // El mundo se dibuja dentro del hueco transparente del frame. El viewport
    // desplaza el origen al hueco y recorta todo lo que se salga, asi el player
    // nunca queda dibujado debajo del chat o de los bordes del frame.
    SDL_Rect game_view = {GAME_VIEW_X, GAME_VIEW_Y, GAME_VIEW_W, GAME_VIEW_H};
    SDL_SetRenderViewport(renderer, &game_view);

    if (tilemap) {
        tilemap->render(camera.get_x(), camera.get_y());
    }

    drawItems();
    if (player) {
        player->draw(camera, player_pov);
    }
    drawEnemies();
    drawOtherPlayers();
    draw_npc_friends();
    draw_spell_animations();
    draw_damage_numbers();

    // Se restaura el viewport completo para dibujar el frame (HUD) y el chat encima.
    SDL_SetRenderViewport(renderer, nullptr);

    if (hud) {
        hud->render();
        hud->display_player_info(own_name);
    }

    // Anillo de progreso del hold de poción, encima del inventario del HUD.
    draw_potion_hold_arc();

    mini_chat->render(GAME_WIDTH, CANVAS_HEIGHT);
    SDL_RenderPresent(renderer);
}

void ClientGUI::load_npc_texture(const std::string& npc_name, const std::string& image_path) {
    SDL_Surface* surf = IMG_Load(image_path.c_str());
    if (surf) {
        SDL_Texture* text = SDL_CreateTextureFromSurface(renderer, surf);
        enemies_textures[npc_name] = text;
        SDL_DestroySurface(surf);
    }
}

void ClientGUI::load_enemies_textures() {
    load_npc_texture("Goblin", "imagenes/goblin1.png");
    load_npc_texture("Spider1", "imagenes/araña1.png");
    load_npc_texture("Spider2", "imagenes/araña2.png");
    load_npc_texture("Spider3", "imagenes/araña3.png");
    load_npc_texture("Skeleton1", "imagenes/esqueleto1.png");
    load_npc_texture("Skeleton2", "imagenes/esqueleto2.png");
    load_npc_texture("Skeleton3", "imagenes/esqueleto3.png");
    load_npc_texture("Golem1", "imagenes/golem1.png");
    load_npc_texture("Golem2", "imagenes/golem2.png");
    load_npc_texture("Golem3", "imagenes/golem3.png");
    load_npc_texture("Zombie", "imagenes/zombie.png");
    load_npc_texture("Orc", "imagenes/orco.png");
}

void ClientGUI::init_draw() {
    initSDL();
    SDL_SetRenderLogicalPresentation(
        renderer, LOGICAL_WIDTH, LOGICAL_HEIGHT,
        SDL_LOGICAL_PRESENTATION_LETTERBOX);

    // Carga provisional solo para tener un tileSize valido al crear el player.
    // El mapa REAL lo carga el primer MSG_ZONE_CHANGE del server (current_zone
    // arranca en un centinela, asi que siempre recarga la zona real). No tocar
    // current_zone aca: dejarlo en el centinela.
    loadMedia(ZONE_CITY);
    // TODO: eliminar esto
    SDL_Surface* enemy_surf = IMG_Load("imagenes/enemigo.png");
    if (!enemy_surf) enemy_surf = IMG_Load("enemigo.png");
    if (enemy_surf) {
        enemy_texture = SDL_CreateTextureFromSurface(renderer, enemy_surf);
        SDL_DestroySurface(enemy_surf);
    }

    load_enemies_textures();

    // Recorte del 1er tile (esquina sup-izq) de cada spritesheet. La grilla es
    // 8 columnas: Goblin 256/8=32px, Spider 512/8=64px.
    enemies_crops["Goblin"] = {0.0f, 0.0f, 32.0f, 32.0f};
    enemies_crops["Spider1"] = {0.0f, 0.0f, 64.0f, 64.0f};
    enemies_crops["Spider2"] = {0.0f, 14.0f, 95.0f, 65.0f};
    enemies_crops["Spider3"] = {20.0f, 69.0f, 84.0f, 57.0f};
    //
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
        // La espada en el piso usa la textura/crop de 101.png.
        floor_item_textures["espada"] = item_texture;
        floor_item_crops["espada"] = {224.0f, 96.0f, 30.0f, 30.0f};
    }

    SDL_Surface* shield_surf = IMG_Load("imagenes/2141.png");
    if (!shield_surf) { shield_surf = IMG_Load("2141.png"); }
    if (shield_surf) {
        SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, shield_surf);
        SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_BLEND);
        SDL_DestroySurface(shield_surf);
        floor_item_textures["escudo"] = tex;
        floor_item_crops["escudo"] = {0.0f, 0.0f, 32.0f, 32.0f};
    }

    // Báculos/varas en el piso: misma imagen que el ícono del HUD. La clave es
    // el id del item (igual que en el server) para que cada uno se vea como su
    // sprite y no caiga al fallback de la espada.
    struct FloorIcon { const char* id; const char* path; float w; float h; };
    const FloorIcon staff_floor_icons[] = {
        {"vara_fresno",      "imagenes/icon_vara_fresno.png",       27.0f,   29.0f},
        {"baculo_nudoso",    "imagenes/icon_baculo_nudoso.png",     30.0f,   29.0f},
        {"baculo_engarzado", "imagenes/icon_baculo_engarzado.png",  34.0f,   40.0f},
        {"flauta_elfica",    "imagenes/icon_flauta_elfica.png",     40.0f,   40.0f},
    };
    for (const auto& fi : staff_floor_icons) {
        SDL_Surface* surf = IMG_Load(fi.path);
        if (!surf) continue;
        SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surf);
        SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_BLEND);
        SDL_DestroySurface(surf);
        if (!tex) continue;
        floor_item_textures[fi.id] = tex;
        floor_item_crops[fi.id] = {0.0f, 0.0f, fi.w, fi.h};
    }

    SDL_Surface* elem_surf = IMG_Load("imagenes/100.png");
    if (!elem_surf) { elem_surf = IMG_Load("100.png"); }
    if (elem_surf) {
        gold_texture = SDL_CreateTextureFromSurface(renderer, elem_surf);
        SDL_DestroySurface(elem_surf);
    }

    // Pociones en el piso: se recortan del mismo spritesheet 100.png que el oro.
    // pocion_vida = botella gris; pocion_mana = botella azul (coords medidas en
    // 100.png). Comparten textura (gold_texture); el cleanup de floor_item_textures
    // ya saltea las que apuntan a otra textura ya liberada.
    if (gold_texture) {
        floor_item_textures["pocion_vida"] = gold_texture;
        floor_item_crops["pocion_vida"] = {392.0f, 159.0f, 16.0f, 26.0f};
        floor_item_textures["pocion_mana"] = gold_texture;
        floor_item_crops["pocion_mana"] = {424.0f, 159.0f, 17.0f, 26.0f};
    }

    hud = std::make_unique<HUD>(renderer, GAME_WIDTH, PANEL_WIDTH, CANVAS_HEIGHT);

    // Spritesheets de efectos de hechizo (animaciones sobre el target).
    load_spell_effects();

    // tile_size viene del TOML
    // el player se escala con el mismo tamano de celda
    int tileSize = tilemap->getTileSize();
    try {
        player = std::make_unique<PlayerDisplay>(renderer, "imagenes/1005.png", tileSize, race);
        player_pov = player->back_pov(ViewDirection::BACK);
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
    // El launcher/login ya corrieron en el ScreenManager: aca solo entramos al
    // juego sobre el window/renderer compartidos. La conexion ya esta abierta y
    // los threads de red ya estan andando.
    try {
        init_draw();
        while (is_running && should_keep_running()) {
            handleEvents();
            update();
            update_potion_hold();  // ¿se cumplieron los 3s del hold de poción?
            draw();
            SDL_Delay(16);
        }
    } catch (const std::exception& e) {
        std::cerr << "ClientGUI error: " << e.what() << std::endl;
    }

    freeSDL();
}
