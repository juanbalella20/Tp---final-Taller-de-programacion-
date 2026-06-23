#include "clientGUI.h"
#include "npcSprite.h"
#include "itemSprite.h"
#include "constants/game_config.h"
#include "paths.h"
#include <SDL3_image/SDL_image.h>
#include <algorithm>
#include <cmath>
#include <iostream>
#include <stdexcept>
#include "scapeWindow.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

ClientGUI::ClientGUI(SDL_Window* window, SDL_Renderer* renderer, TTF_Font* font,
    Queue<ClientCmd>& outgoing, Queue<GameMsg>& receiving, const std::string& player_name,
    const std::string& player_race, const std::string& player_clan)
    : window(window), renderer(renderer), event{}, font(font), texture_loader(window, renderer),
      is_running(false), mini_chat(nullptr), parser(), outgoing(outgoing), receiving(receiving),
      hud(nullptr), own_name(player_name), race(player_race), own_clan(player_clan),
      player(nullptr), tilemap(nullptr),
      zone_music(nullptr),
      camera((float)GAME_VIEW_W, (float)GAME_VIEW_H),
      current_zone(static_cast<Zone>(0xFF)),
      selected_npc_tile_x(-1), selected_npc_tile_y(-1) {}

ClientGUI::~ClientGUI() {
    freeSDL();
}

void ClientGUI::initSDL() {
    SDL_SetRenderLogicalPresentation(renderer, LOGICAL_WIDTH, LOGICAL_HEIGHT, SDL_LOGICAL_PRESENTATION_LETTERBOX);

    texture_loader.load_game_logo();

    mini_chat = std::make_unique<MiniChat>(renderer, GAME_WIDTH, PANEL_WIDTH, CANVAS_HEIGHT);
    zone_music = std::make_unique<ZoneMusicPlayer>();
    sfx = std::make_unique<SoundPlayer>();
}

const char* ClientGUI::weapon_sound_path(const std::string& weapon_id) {
    if (weapon_id == "espada")
        return "client/audio/sounds/freesound_community-hit-swing-sword-small-2-95566_1_.ogg";
    if (weapon_id == "hacha")
        return "client/audio/sounds/sonido-hacha.ogg";
    if (weapon_id == "martillo")
        return "client/audio/sounds/sonido-martillo.ogg";
    if (weapon_id == "arco_simple" || weapon_id == "arco_compuesto")
        return "client/audio/sounds/arco-flecha.ogg";
    if (weapon_id == "baculo_nudoso")
        return "client/audio/sounds/bomba.ogg";
    if (weapon_id == "baculo_engarzado")
        return "client/audio/sounds/bombav2.ogg";
    return nullptr;
}

void ClientGUI::loadMedia(Zone zone) {
   // Ruta del .bin desde config.toml (zones.<nombre>.map): la MISMA fuente que
    // usa el servidor, asi nunca se desincronizan los mapas de ambos lados.
    const std::string& map_path = GameConfig::instance().zone_map_path(zone);
    if (map_path.empty())
        return;
    tilemap = std::make_unique<TileMap>(renderer);
    tilemap->load_map_bin(map_path);
}

void ClientGUI::freeSDL() {
    zone_music.reset();

    player.reset();
    tilemap.reset();
    hud.reset();
    shop_window.reset();

    texture_loader.freeSDL();
}

std::vector<int> ClientGUI::translate_tile_to_coord(int pixel_x, int pixel_y) const {
    int tileSize = tilemap ? tilemap->getTileSize() : 64;

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
    cmd.set_item_id(item_uid);
    outgoing.push(cmd);
}

bool ClientGUI::potion_slot_at(int mx, int my, uint64_t& out_uid,
                               SDL_FRect& out_rect) const {
    if (!hud) return false;
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
        potion_hold_sent = true;

        if (sfx) sfx->stop_hold();
    }
}

void ClientGUI::draw_potion_hold_arc() {
    if (!potion_hold_active || potion_hold_sent) return;
    uint64_t elapsed = SDL_GetTicks() - potion_hold_start_ms;
    float progress = static_cast<float>(elapsed) / POTION_HOLD_MS;
    if (progress > 1.0f) progress = 1.0f;

    float cx = potion_hold_slot_rect.x + potion_hold_slot_rect.w / 2.0f;
    float cy = potion_hold_slot_rect.y + potion_hold_slot_rect.h / 2.0f;
    float radius = (potion_hold_slot_rect.w / 2.0f) + 4.0f;

    const int total_segments = 48;
    int lit = static_cast<int>(progress * total_segments);

    for (int i = 0; i < total_segments; ++i) {
        float frac = static_cast<float>(i) / total_segments;
        float angle = -static_cast<float>(M_PI) / 2.0f + frac * 2.0f * static_cast<float>(M_PI);
        float px = cx + radius * std::cos(angle);
        float py = cy + radius * std::sin(angle);
        if (i < lit) {
            SDL_SetRenderDrawColor(renderer, 80, 220, 120, 255);
        } else {
            SDL_SetRenderDrawColor(renderer, 40, 60, 40, 120);
        }

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

    if (player && equipped_spell_id == "flauta_elfica") {
        spawn_spell_animation(equipped_spell_id, player->getTileX(), player->getTileY());
    }
}

void ClientGUI::handleEvents() {
    while (SDL_PollEvent(&event)) {
        if (scape_window && scape_window->handle_event(event)) {
            continue;
        }

        if (shop_window && shop_window->handle_event(event)) {
            continue;
        }

        if (mini_chat->handle_event(event)) {
            continue;
        }

        switch (event.type) {
            case SDL_EVENT_QUIT: {
                ClientCmd logout_cmd;
                logout_cmd.set_message_type(MSG_LOGOUT);
                outgoing.try_push(logout_cmd);
                is_running = false;
                break;
            }
            case SDL_EVENT_KEY_DOWN:
                if (event.key.scancode == SDL_SCANCODE_T) {
                    if (!(mini_chat->is_active())) {
                        mini_chat->toggle_active();
                    }
                    break;
                }
                switch (event.key.scancode) {
                    case SDL_SCANCODE_ESCAPE: {
                        if (scape_window) scape_window->open();
                        break;
                    }
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

                if (event.button.button != SDL_BUTTON_LEFT) break;

                {
                    uint64_t uid = 0;
                    SDL_FRect slot_rect;
                    if (potion_slot_at(mx, my, uid, slot_rect)) {
                        potion_hold_active = true;
                        potion_hold_sent = false;
                        potion_hold_uid = uid;
                        potion_hold_start_ms = SDL_GetTicks();
                        potion_hold_slot_rect = slot_rect;

                        if (sfx) {
                            sfx->play_hold(
                                "client/audio/sounds/Mini-escudo-sonido-fortnite.ogg");
                        }
                        break;
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
                    auto coords = translate_tile_to_coord(mx, my);
                    selectCoord(coords[0], coords[1]);
                }
                break;
            }
            case SDL_EVENT_MOUSE_BUTTON_UP: {
                if (event.button.button == SDL_BUTTON_LEFT) {
                    if (potion_hold_active && !potion_hold_sent && sfx) {
                        sfx->stop_hold();
                    }
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
                        if (zone_music) {
                            zone_music->play_zone(z);
                        }
                        current_zone = z;
                    }

                    other_players.clear();
                    player->setTilePosition(msg.get_coord_x(), msg.get_coord_y());
                    break;
                }
                case MSG_REGISTER: {
                    world_map = msg.get_map();

                    last_known_hp = msg.get_hp();
                    has_known_hp = true;
                    if (hud) {
                        hud->set_inventory(msg.get_items());
                        hud->set_gold(msg.get_gold());
                        hud->set_hp(msg.get_hp());
                        hud->set_max_hp(msg.get_max_hp());
                        hud->set_max_mana(msg.get_max_mana());
                        hud->set_xp(msg.get_xp());
                        hud->set_max_xp(msg.get_max_xp());
                        hud->set_mana(msg.get_mana());
                        hud->set_level(msg.get_level());
                    }
                    if (player) {
                        player->setTilePosition(msg.get_coord_x(), msg.get_coord_y());
                        player->set_ghost(msg.get_ghost());
                        player_pov = player->front_pov(ViewDirection::FRONT);
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
                    set_npcs_snapshot(msg.get_npcs());
                    break;
                case MSG_ITEMS_SNAPSHOT:
                    items_on_floor = msg.get_items_on_floor();
                    break;
                case MSG_PLAYERS_SNAPSHOT: {
                    for (const auto& incoming : msg.get_players()) {
                        auto it = std::find_if(other_players.begin(), other_players.end(),
                            [&incoming](const PlayerInfo& p) { return p.name == incoming.name; });
                        if (it != other_players.end()) {
                            *it = incoming;
                        } else {
                            other_players.push_back(incoming);
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

                    if (mover.empty() || mover == own_name) {
                        player->setTilePosition(x, y);
                        switch (msg.get_direction()) {
                            case DIR_NORTH:
                                player_pov = player->back_pov(ViewDirection::BACK);
                                break;
                            case DIR_SOUTH:
                                player_pov = player->front_pov(ViewDirection::FRONT);
                                break;
                            case DIR_EAST:
                                player_pov = player->right_pov(ViewDirection::RIGHT);
                                break;
                            case DIR_WEST:
                                player_pov = player->left_pov(ViewDirection::LEFT);
                                break;
                            default: break;
                        }
                    } else {
                        auto it = std::find_if(other_players.begin(), other_players.end(),
                            [&mover](const PlayerInfo& p) { return p.name == mover; });
                        Direction dir = static_cast<Direction>(msg.get_direction());
                        if (it != other_players.end()) {
                            bool ghost = it->ghost;
                            bool moved = (it->x != x || it->y != y);
                            it->moving = moved;
                            it->update_frame = true;
                            it->x = x;
                            it->y = y;
                            it->direction = dir;
                            it->ghost = ghost;
                        } else {
                            PlayerInfo pi{mover, msg.get_race(), 0, x, y, dir};
                            pi.ghost = false;
                            pi.moving = false;
                            pi.update_frame = true;
                            other_players.push_back(pi);
                        }
                    }
                    break;
                }
                case MSG_PRIVATE:

                    chat_inbox.push(msg.get_chat_content());
                    break;
                case MSG_MEDITATE:
                case MSG_RESURRECT: {
                    if (msg.get_player_name() == own_name) {
                        player->set_ghost(false);
                        player_pov = player->front_pov(ViewDirection::FRONT);
                    } else {
                        for (auto& p : other_players) {
                            if (p.name == msg.get_player_name()) {
                                p.ghost = false;
                                break;
                            }
                        }
                    }
                    chat_inbox.push(msg.get_chat_content());
                    break;
                }
                case MSG_LIST:

                    if (shop_window) shop_window->open(msg.get_items());
                    break;
                case MSG_CURE:
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
                        player_pov = player->front_pov(ViewDirection::FRONT);
                    } else {
                        for (auto& p : other_players) {
                            if (p.name == msg.get_player_name()) {
                                p.ghost = true;
                                p.has_equipped_weapon = false;
                                break;
                            }
                        }
                    }
                    chat_inbox.push(msg.get_chat_content());
                    break;
                }
                case MSG_CHEAT_RESPAWN: {
                    if (msg.get_player_name() == own_name) {
                        player->set_ghost(false);
                        player_pov = player->front_pov(ViewDirection::FRONT);
                    } else {
                        for (auto& p : other_players) {
                            if (p.name == msg.get_player_name()) {
                                p.ghost = false;
                                break;
                            }
                        }
                    }
                    chat_inbox.push(msg.get_chat_content());
                    break;
                }
                case MSG_CLAN_UPDATE:

                    if (msg.get_player_name() == own_name) {
                        own_clan = msg.get_chat_content();
                    }
                    break;
                case MSG_GOLD:
                    if (hud) hud->set_gold(msg.get_gold());
                    break;
                case MSG_HP:
                    if (hud) hud->set_hp(msg.get_hp());
                    note_local_hp(msg.get_hp());
                    break;
                case MSG_ATTACK:
                    if (msg.get_damage() > 0) {
                        damage_numbers.push_back({
                            msg.get_coord_x(), msg.get_coord_y(),
                            msg.get_damage(), SDL_GetTicks() + DMG_MS
                        });
                    }

                    if (sfx) {
                        const char* sound = weapon_sound_path(equipped_weapon_id);
                        if (sound != nullptr) sfx->play(sound);
                    }

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

                    if (msg.get_player_name() == own_name) {
                        if (player) {
                            player->set_equipped_weapon(msg.get_equipped());

                            player->set_equipped_items(msg.get_equipped_ids());
                        }
                        if (hud) hud->set_equipped_by_uids(msg.get_equipped_uids());

                        equipped_spell_id.clear();
                        for (const auto& id : msg.get_equipped_ids()) {
                            if (spell_effects.find(id) != spell_effects.end()) {
                                equipped_spell_id = id;
                                break;
                            }
                        }

                        equipped_weapon_id.clear();
                        for (const auto& id : msg.get_equipped_ids()) {
                            if (weapon_sound_path(id) != nullptr) {
                                equipped_weapon_id = id;
                                break;
                            }
                        }

                        std::set<std::string> new_defense_uids;
                        if (hud) {
                            const auto& inv = hud->get_inventory();
                            for (const auto& uid : msg.get_equipped_uids()) {
                                for (const auto& item : inv) {
                                    if (std::to_string(item.get_uid()) != uid)
                                        continue;
                                    uint8_t t = item.get_type();
                                    if (t == 1 || t == 2 || t == 3)
                                        new_defense_uids.insert(uid);
                                    break;
                                }
                            }
                        }
                        bool equipped_new_defense = false;
                        for (const auto& uid : new_defense_uids) {
                            if (equipped_defense_uids.count(uid) == 0) {
                                equipped_new_defense = true;
                                break;
                            }
                        }
                        if (equipped_new_defense && sfx) {
                            sfx->play("client/audio/sounds/equipacion-de-armadura.ogg");
                        }
                        equipped_defense_uids = std::move(new_defense_uids);
                    } else {
                        for (auto& p : other_players) {
                            if (p.name == msg.get_player_name()) {
                                p.has_equipped_weapon = msg.get_equipped();
                                p.equipped_ids = msg.get_equipped_ids();
                                break;
                            }
                        }
                    }
                    break;
                case MSG_UPDATE_LEVEL: {
                    if (hud) {
                        hud->set_level(msg.get_level());
                        hud->set_max_xp(msg.get_max_xp());
                        hud->set_max_hp(msg.get_max_hp());
                        hud->set_max_mana(msg.get_max_mana());
                    }
                    break;
                }
                case MSG_DEATH: {
                    if (msg.get_player_name() == own_name) {
                        player->set_ghost(true);
                        player_pov = player->front_pov(ViewDirection::FRONT);
                    } else {
                        for (auto& p : other_players) {
                            if (p.name == msg.get_player_name()) {
                                p.ghost = true;
                                p.has_equipped_weapon = false;
                                break;
                            }
                        }
                    }
                    break;
                }
                case MSG_PLAYER_LEFT: {
                    const std::string& gone = msg.get_player_name();
                    other_players.erase(
                        std::remove_if(other_players.begin(), other_players.end(),
                                       [&gone](const PlayerInfo& p) { return p.name == gone; }),
                        other_players.end());
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

void ClientGUI::set_npcs_snapshot(const std::vector<NpcInfo>& next_npcs) {
    auto direction_from_delta = [](int dx, int dy, Direction fallback) {
        if (dx != 0 && std::abs(dx) >= std::abs(dy)) return dx > 0 ? DIR_EAST : DIR_WEST;
        if (dy != 0) return dy > 0 ? DIR_SOUTH : DIR_NORTH;
        return fallback;
    };

    if (npc_walk_frames.size() != next_npcs.size()) {
        npc_walk_frames.assign(next_npcs.size(), 0);
        npc_draw_directions.assign(next_npcs.size(), DIR_SOUTH);
    }

    for (size_t i = 0; i < next_npcs.size(); ++i) {
        Direction direction = next_npcs[i].direction;
        bool moved = false;

        if (i < npcs.size() && npcs[i].name == next_npcs[i].name) {
            const int dx = next_npcs[i].x - npcs[i].x;
            const int dy = next_npcs[i].y - npcs[i].y;
            moved = dx != 0 || dy != 0;
            if (moved) direction = direction_from_delta(dx, dy, direction);
        }

        npc_walk_frames[i] = moved ? npc_walk_frames[i] + 1 : 0;
        npc_draw_directions[i] = direction;
    }

    npcs = next_npcs;
}

void ClientGUI::drawEnemies() {
    if (!tilemap) return;
    const int tileSize = tilemap->getTileSize();
    for (size_t i = 0; i < npcs.size(); ++i) {
        const auto& npc = npcs[i];
        if (npc.type == "seller" || npc.type == "banker" || npc.type == "priest") continue;
        SDL_Texture* npc_tex = texture_loader.get_texture_of_npc_enemy(npc.name);

        const int frame = i < npc_walk_frames.size() ? npc_walk_frames[i] : 0;
        const Direction direction =
            i < npc_draw_directions.size() ? npc_draw_directions[i] : npc.direction;
        NpcSprite ns(renderer, npc_tex, npc.x, npc.y, tileSize, npc.name,
                     frame);
        SDL_FRect pov = ns.front_pov(npc.name);
        switch (direction) {
            case DIR_NORTH:
                pov = ns.back_pov(npc.name);
                break;
            case DIR_SOUTH:
                pov = ns.front_pov(npc.name);
                break;
            case DIR_EAST:
                pov = ns.right_pov(npc.name);
                break;
            case DIR_WEST:
                pov = ns.left_pov(npc.name);
                break;
            default: break;
        }
        ns.draw(camera, pov);
    }
}

void ClientGUI::draw_npc_friends() {
    if (!tilemap) return;
    const int tileSize = tilemap->getTileSize();
    for (const auto& npc : npcs) {
        SDL_Texture* tex = nullptr;
        SDL_FRect crop = {2.0f, 0.0f, 23.0f, 45.0f};
        if (npc.type == "seller") {
            tex = texture_loader.get_seller_texture();
        } else if (npc.type == "banker") {
            tex = texture_loader.get_banker_texture();
            crop.w = 24.0f;
            crop.h = 47.0f;
        } else if (npc.type == "priest") tex = texture_loader.get_priest_texture();
        else continue;
        if (!tex) continue;
        NpcSprite(renderer, tex, npc.x, npc.y, tileSize).draw(camera, crop);
    }
}

void ClientGUI::drawOtherPlayers() {
    if (!tilemap) return;
    const int tileSize = tilemap->getTileSize();

    for (auto it = other_player_displays.begin(); it != other_player_displays.end();) {
        const std::string& cached_name = it->first;
        bool still_here = std::any_of(other_players.begin(), other_players.end(),
            [&cached_name](const PlayerInfo& p) { return p.name == cached_name; });
        it = still_here ? std::next(it) : other_player_displays.erase(it);
    }

    for (auto& p : other_players) {
        auto it = other_player_displays.find(p.name);
        if (it == other_player_displays.end()) {
            try {
                it = other_player_displays
                         .try_emplace(p.name, renderer, "imagenes/1005.png", tileSize, p.race)
                         .first;
            } catch (const std::runtime_error& e) {
                std::cout << "[DEBUG: drawOtherPlayers] textura fallida: " << e.what() << std::endl;
                continue;
            }
        }
        PlayerDisplay& pd = it->second;
        pd.setTilePosition(p.x, p.y);
        pd.update_motion();
        pd.set_equipped_weapon(p.has_equipped_weapon);
        pd.set_equipped_items(p.equipped_ids);
        pd.set_ghost(p.ghost);

        if (p.update_frame) {
            if (!p.moving) {
                pd.reset_frame();
            }
            SDL_FRect pov;
            switch (p.direction) {
                case DIR_NORTH:
                    pov = pd.back_pov(ViewDirection::BACK);
                    break;
                case DIR_SOUTH:
                    pov = pd.front_pov(ViewDirection::FRONT);
                    break;
                case DIR_EAST:
                    pov = pd.right_pov(ViewDirection::RIGHT);
                    break;
                case DIR_WEST:
                    pov = pd.left_pov(ViewDirection::LEFT);
                    break;
                default: break;
            }
            other_players_povs[p.name] = pov;
            p.update_frame = false;
        }

        bool mismo_clan = !own_clan.empty() && p.clan_name == own_clan;
        SDL_Color color = mismo_clan ? SDL_Color{80, 255, 120, 255}
                                     : SDL_Color{255, 80, 80, 255};
        draw_player_name(p.name, p.x, p.y, color);
        pd.draw(camera, other_players_povs[p.name]);
    }
}

void ClientGUI::draw_player_name(const std::string& name, int tile_x, int tile_y,
                                 SDL_Color color) {
    if (!font || name.empty() || !tilemap) return;

    SDL_Color outline_color = {0, 0, 0, 255};
    SDL_Surface* fill_surf = TTF_RenderText_Blended(font, name.c_str(), 0, color);
    if (!fill_surf) return;
    SDL_Surface* outline_surf = TTF_RenderText_Blended(font, name.c_str(), 0, outline_color);

    SDL_Texture* fill_tex = SDL_CreateTextureFromSurface(renderer, fill_surf);
    SDL_Texture* outline_tex = outline_surf ? SDL_CreateTextureFromSurface(renderer, outline_surf)
                                            : nullptr;
    SDL_DestroySurface(fill_surf);
    if (outline_surf) SDL_DestroySurface(outline_surf);
    if (!fill_tex) {
        if (outline_tex) SDL_DestroyTexture(outline_tex);
        return;
    }

    float text_w = 0.0f, text_h = 0.0f;
    SDL_GetTextureSize(fill_tex, &text_w, &text_h);

    const int tileSize = tilemap->getTileSize();

    float world_x = static_cast<float>(tile_x * tileSize);
    float world_y = static_cast<float>(tile_y * tileSize);

    float screen_x = camera.world_to_screen_x(world_x) + (tileSize - text_w) / 2.0f;
    float screen_y = camera.world_to_screen_y(world_y) - text_h - tileSize * 0.5f;

    if (outline_tex) {
        const float OUTLINE = 1.0f;
        const float offsets[8][2] = {
            {-OUTLINE, -OUTLINE}, {0.0f, -OUTLINE}, {OUTLINE, -OUTLINE},
            {-OUTLINE,  0.0f},                      {OUTLINE,  0.0f},
            {-OUTLINE,  OUTLINE}, {0.0f,  OUTLINE}, {OUTLINE,  OUTLINE},
        };
        for (const auto& off : offsets) {
            SDL_FRect odst = {screen_x + off[0], screen_y + off[1], text_w, text_h};
            SDL_RenderTexture(renderer, outline_tex, nullptr, &odst);
        }
        SDL_DestroyTexture(outline_tex);
    }

    SDL_FRect dst = {screen_x, screen_y, text_w, text_h};
    SDL_RenderTexture(renderer, fill_tex, nullptr, &dst);
    SDL_DestroyTexture(fill_tex);
}

void ClientGUI::load_spell_effects() {
    struct Def { const char* id; int fw; int fh; int cols; int count; };
    const Def defs[] = {
        {"flauta_elfica", 204, 204, 5, 10},

        {"vara_fresno", 128, 128, 4, 15},

        {"baculo_nudoso", 128, 128, 4, 16},

        {"baculo_engarzado", 128, 128, 4, 16},
    };
    for (const auto& d : defs) {
        spell_effects[d.id] = {d.fw, d.fh, d.cols, d.count};
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
    constexpr uint64_t FRAME_MS = 55;

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

        const float size = tileSize * 1.6f;
        const float wx = static_cast<float>(a.tile_x * tileSize);
        const float wy = static_cast<float>(a.tile_y * tileSize);
        const float sx = camera.world_to_screen_x(wx) + (tileSize - size) / 2.0f;
        const float sy = camera.world_to_screen_y(wy) + (tileSize - size) / 2.0f;
        SDL_FRect dst{sx, sy, size, size};
        SDL_Texture* spell_tex = texture_loader.get_texture_of_spell(a.effect_id);
        SDL_RenderTexture(renderer, spell_tex, &src, &dst);
    }
}

void ClientGUI::draw_damage_numbers() {
    if (!tilemap || !font) return;
    const int tileSize = tilemap->getTileSize();
    const SDL_Color color = {255, 0, 0, 255};
    const uint64_t now = SDL_GetTicks();

    damage_numbers.erase(
        std::remove_if(damage_numbers.begin(), damage_numbers.end(),
                       [now](const DamageNumber& d) { return now >= d.expire_ms; }),
        damage_numbers.end());

    for (const auto& d : damage_numbers) {
        const std::string text = std::to_string(d.value);
        SDL_Surface* surf = TTF_RenderText_Blended(font, text.c_str(), 0, color);
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

void ClientGUI::note_local_hp(uint32_t new_hp) {
    if (!has_known_hp) {
        last_known_hp = new_hp;
        has_known_hp = true;
        return;
    }

    if (new_hp < last_known_hp) {
        damage_flash_until_ms = SDL_GetTicks() + DAMAGE_FLASH_MS;
    }
    last_known_hp = new_hp;
}

void ClientGUI::draw_damage_flash() {
    const uint64_t now = SDL_GetTicks();
    if (now >= damage_flash_until_ms) return;

    const float remaining = static_cast<float>(damage_flash_until_ms - now);
    const float intensity = remaining / static_cast<float>(DAMAGE_FLASH_MS);

    const float w = static_cast<float>(GAME_VIEW_W);
    const float h = static_cast<float>(GAME_VIEW_H);

    const int   bands = 18;
    const float max_depth = h * 0.28f;

    const Uint8 peak_alpha = static_cast<Uint8>(70.0f * intensity);
    if (peak_alpha == 0) return;

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    for (int i = 0; i < bands; ++i) {
        const float t = static_cast<float>(i) / static_cast<float>(bands - 1);
        const float depth = max_depth * t;

        const float fade = (1.0f - t) * (1.0f - t);
        const Uint8 a = static_cast<Uint8>(peak_alpha * fade);
        if (a == 0) continue;

        SDL_SetRenderDrawColor(renderer, 190, 0, 0, a);

        const float thickness = max_depth / static_cast<float>(bands) + 1.0f;
        SDL_FRect top    = {0.0f,            depth,                  w, thickness};
        SDL_FRect bottom = {0.0f,            h - depth - thickness,  w, thickness};
        SDL_FRect left   = {depth,           0.0f,  thickness, h};
        SDL_FRect right  = {w - depth - thickness, 0.0f, thickness, h};
        SDL_RenderFillRect(renderer, &top);
        SDL_RenderFillRect(renderer, &bottom);
        SDL_RenderFillRect(renderer, &left);
        SDL_RenderFillRect(renderer, &right);
    }
}

void ClientGUI::drawItems() {
    if (!tilemap) return;
    const int tileSize = tilemap->getTileSize();
    for (const auto& item : items_on_floor) {
        SDL_Texture* tex = nullptr;
        SDL_FRect crop = {0.0f, 0.0f, 0.0f, 0.0f};
        float scale = 0.8f;
        if (item.type == "gold") {
            tex = texture_loader.get_gold_texture();
            if (!tex) continue;
        } else {
            tex = texture_loader.get_texture_of_item(item.type);
            if (!tex) continue;
            if (item.type == "pocion_vida" || item.type == "pocion_mana") scale = 0.6f;
            if (item.type == "escudo_hierro") scale = 0.4f;
        }
        ItemSprite(renderer, tex, item.x, item.y, tileSize, scale).draw(camera, crop);
    }
}

void ClientGUI::draw() {
    player->update_motion();

    const int tileSize = tilemap ? tilemap->getTileSize() : TILE_SIZE;
    camera.center_on(player->get_x() + tileSize / 2.0f,
                     player->get_y() + tileSize / 2.0f);
    if (tilemap) {
        camera.clamp_to(static_cast<float>(tilemap->getPixelWidth()),
                        static_cast<float>(tilemap->getPixelHeight()));
    }

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    SDL_Rect game_view = {GAME_VIEW_X, GAME_VIEW_Y, GAME_VIEW_W, GAME_VIEW_H};
    SDL_SetRenderViewport(renderer, &game_view);

    if (tilemap) {
        tilemap->render(camera.get_x(), camera.get_y(),
                        LAYER_GROUND, LAYER_ABOVE_PLAYER);
    }

    drawItems();
    draw_npc_friends();
    drawEnemies();
    if (player) {
        player->draw(camera, player_pov);
    }
    drawOtherPlayers();

    if (tilemap) {
        tilemap->render(camera.get_x(), camera.get_y(),
                        LAYER_ABOVE_PLAYER, LAYER_LAST);
    }

    draw_spell_animations();
    draw_damage_numbers();

    draw_damage_flash();

    SDL_SetRenderViewport(renderer, nullptr);

    if (hud) {
        hud->render();
        hud->display_player_info(own_name);
    }

    draw_potion_hold_arc();

    mini_chat->render(GAME_WIDTH, CANVAS_HEIGHT);

    if (shop_window) shop_window->render();

    if (scape_window) scape_window->render();

    SDL_RenderPresent(renderer);
}

void ClientGUI::init_draw() {
    initSDL();
    SDL_SetRenderLogicalPresentation(
        renderer, LOGICAL_WIDTH, LOGICAL_HEIGHT,
        SDL_LOGICAL_PRESENTATION_LETTERBOX);

    loadMedia(ZONE_CITY);

    texture_loader.load_npcs_enemies();
    texture_loader.load_items();
    texture_loader.load_gold();
    texture_loader.load_npcs_friendlies();

    hud = std::make_unique<HUD>(renderer, GAME_WIDTH, PANEL_WIDTH, CANVAS_HEIGHT);

    shop_window = std::make_unique<ShopWindow>(renderer, font,
                                               LOGICAL_WIDTH, LOGICAL_HEIGHT);

    scape_window = std::make_unique<ScapeWindow>(renderer,
                                                 LOGICAL_WIDTH, LOGICAL_HEIGHT);

    texture_loader.load_spell_effects();
    load_spell_effects();

    int tileSize = tilemap->getTileSize();
    try {
        player = std::make_unique<PlayerDisplay>(renderer, "imagenes/1005.png", tileSize, race);
        player_pov = player->front_pov(ViewDirection::FRONT);
    } catch (const std::runtime_error& e) {
        std::cout << "[DEBUG] imagenes/1005.png failed: " << e.what() << std::endl;
    }
    if (!player) {
        throw std::runtime_error(
            "No se pudo crear el PlayerDisplay local (raza='" + race + "')");
    }

    player->setTilePosition(0, 0);
    draw_npc_friends();

    is_running = true;
    SDL_Delay(100);
}

void ClientGUI::run() {
    try {
        init_draw();
        while (is_running && should_keep_running()) {
            handleEvents();
            if (scape_window && scape_window->consume_exit_confirmed()) {
                ClientCmd logout_cmd;
                logout_cmd.set_message_type(MSG_LOGOUT);
                outgoing.try_push(logout_cmd);
                is_running = false;
                break;
            }
            update();
            update_potion_hold();
            draw();
            SDL_Delay(16);
        }
    } catch (const std::exception& e) {
        std::cerr << "ClientGUI error: " << e.what() << std::endl;
    }

    freeSDL();
}
