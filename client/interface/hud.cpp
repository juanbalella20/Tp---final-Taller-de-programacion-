#include "hud.h"

HUD::HUD(SDL_Renderer* gui_renderer,
         float game_width, float panel_width, float canvas_height)
    : gui_renderer(gui_renderer),
      font(TTF_OpenFont("fonts/Roboto-VariableFont_wdth,wght.ttf", 16)),
      inventory{},
      attack_btn{game_width + 10.0f, canvas_height - 60.0f, panel_width - 20.0f, 40.0f},
      show_attack_button(false),
      player_gold(0),
      player_hp(0),
      player_xp(0),
      player_mana(0),
      game_width(game_width),
      panel_width(panel_width),
      canvas_height(canvas_height) {
    inventory.panel = {game_width, 0.0f, panel_width, canvas_height};
    load_textures();
}

HUD::~HUD() {
    if (inventory.bg_texture) {
        SDL_DestroyTexture(inventory.bg_texture);
    }
    for (auto& kv : inventory.items_textures) {
        if (kv.second) SDL_DestroyTexture(kv.second);
    }
    if (font) {
        TTF_CloseFont(font);
    }
}

void HUD::set_inventory(const std::vector<ItemInfo>& items) {
    inventory.items = items;
}

void HUD::set_attack_button_visible(bool visible) {
    show_attack_button = visible;
}

void HUD::set_gold(uint32_t amount) {
    player_gold = amount;
}

void HUD::set_hp(uint32_t hp) {
    player_hp = hp;
}

void HUD::set_xp(uint32_t xp) {
    player_xp = xp;
}

void HUD::set_mana(uint32_t mana) {
    player_mana = mana;
}

void HUD::drawAttackButton() {
    if (!show_attack_button) return;

    SDL_SetRenderDrawColor(gui_renderer, 200, 50, 50, 255);
    SDL_RenderFillRect(gui_renderer, &attack_btn);
    SDL_SetRenderDrawColor(gui_renderer, 255, 255, 255, 255);
    SDL_RenderRect(gui_renderer, &attack_btn);

    if (font) {
        SDL_Color white = {255, 255, 255, 255};
        SDL_Surface* surf = TTF_RenderText_Solid(font, "PEGAR", 0, white);
        if (surf) {
            SDL_Texture* tex = SDL_CreateTextureFromSurface(gui_renderer, surf);
            SDL_FRect dst = {attack_btn.x + (attack_btn.w - surf->w) / 2.0f,
                             attack_btn.y + (attack_btn.h - surf->h) / 2.0f,
                             static_cast<float>(surf->w), static_cast<float>(surf->h)};
            SDL_RenderTexture(gui_renderer, tex, nullptr, &dst);
            SDL_DestroyTexture(tex);
            SDL_DestroySurface(surf);
        }
    }
}

void HUD::drawInventoryPanel() {
    // Fondo del panel: textura si esta disponible, color solido como fallback
    if (inventory.bg_texture) {
        SDL_RenderTexture(gui_renderer, inventory.bg_texture, nullptr, &inventory.panel);
    } else {
        SDL_SetRenderDrawColor(gui_renderer, 30, 30, 30, 255);
        SDL_RenderFillRect(gui_renderer, &inventory.panel);
        SDL_SetRenderDrawColor(gui_renderer, 180, 180, 180, 255);
        SDL_RenderRect(gui_renderer, &inventory.panel);
    }

    // Titulo "Inventario"
    if (font) {
        SDL_Color white = {255, 255, 255, 255};
        SDL_Surface* surf = TTF_RenderText_Solid(font, "Inventario", 0, white);
        if (surf) {
            SDL_Texture* tex = SDL_CreateTextureFromSurface(gui_renderer, surf);
            SDL_FRect dst = {game_width + 10.0f, 10.0f,
                             static_cast<float>(surf->w), static_cast<float>(surf->h)};
            SDL_RenderTexture(gui_renderer, tex, nullptr, &dst);
            SDL_DestroyTexture(tex);
            SDL_DestroySurface(surf);
        }
    }

    // Items del inventario en slots
    const float SLOT_SIZE = 48.0f;
    const float SLOT_MARGIN = 8.0f;
    float slot_x = game_width + SLOT_MARGIN;
    float slot_y = 40.0f;

    for (const auto& item : inventory.items) {
        SDL_FRect slot_rect = {slot_x, slot_y, SLOT_SIZE, SLOT_SIZE};
        SDL_SetRenderDrawBlendMode(gui_renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(gui_renderer, 60, 60, 60, 120);
        SDL_RenderFillRect(gui_renderer, &slot_rect);
        SDL_SetRenderDrawColor(gui_renderer, 120, 120, 120, 180);
        SDL_RenderRect(gui_renderer, &slot_rect);

        auto it = inventory.items_textures.find(item.get_id());
        if (it != inventory.items_textures.end() && it->second != nullptr) {
            SDL_Texture* icon = it->second;
            
            float tex_w, tex_h;
            SDL_GetTextureSize(icon, &tex_w, &tex_h); 
            const float PADDING = 4.0f;
            float max_size = SLOT_SIZE - (PADDING * 2.0f);
            float scale = SDL_min(max_size / tex_w, max_size / tex_h);
            float final_w = tex_w * scale;
            float final_h = tex_h * scale;
            
            SDL_FRect icon_dst = {
                slot_x + (SLOT_SIZE - final_w) / 2.0f,
                slot_y + (SLOT_SIZE - final_h) / 2.0f,
                final_w,
                final_h
            };
            
            SDL_RenderTexture(gui_renderer, icon, nullptr, &icon_dst);
        }

        slot_x += SLOT_SIZE + SLOT_MARGIN;
        if (slot_x + SLOT_SIZE > game_width + panel_width - SLOT_MARGIN) {
            slot_x = game_width + SLOT_MARGIN;
            slot_y += SLOT_SIZE + SLOT_MARGIN;
        }
    }
}

void HUD::draw_stat(const std::string& text, float pos_y) {
    SDL_Color color = {255, 255, 255, 255};
    SDL_Surface* surf = TTF_RenderText_Solid(font, text.c_str(), 0, color);

    if (surf) {
        SDL_Texture* tex = SDL_CreateTextureFromSurface(gui_renderer, surf);

        if (tex) {
            float pos_x = game_width + 15.0f;

            SDL_FRect dest = { pos_x, pos_y, static_cast<float>(surf->w), static_cast<float>(surf->h)};
            SDL_RenderTexture(gui_renderer, tex, nullptr, &dest);
            SDL_DestroyTexture(tex);
        }
        SDL_DestroySurface(surf);
    }
}

void HUD::draw_gold() {
    if (!font) return;

    std::string gold_text = "Oro: " + std::to_string(player_gold);

    draw_stat(gold_text, 400.0f);
}

void HUD::draw_hp() {
    if (!font) return;

    std::string hp_text = "Vidas: " + std::to_string(player_hp);

    draw_stat(hp_text, 420.0f);
}

void HUD::draw_xp() {
    if (!font) return;

    std::string xp_text = "Experiencia: " + std::to_string(player_xp);

    draw_stat(xp_text, 440.0f);
}

void HUD::draw_mana() {
    if (!font) return;

    std::string mana_text = "Mana: " + std::to_string(player_mana);

    draw_stat(mana_text, 460.0f);
}

void HUD::load_textures() {
    SDL_Surface* inv_bg_surf = IMG_Load("imagenes/inventory-bg..png");
    if (inv_bg_surf) {
        inventory.bg_texture = SDL_CreateTextureFromSurface(gui_renderer, inv_bg_surf);
        SDL_DestroySurface(inv_bg_surf);
    }

    SDL_Surface* sword_surf = IMG_Load("imagenes/espada.png");
    if (sword_surf) {
        SDL_Texture* tex = SDL_CreateTextureFromSurface(gui_renderer, sword_surf);
        SDL_SetTextureBlendMode(tex, SDL_SCALEMODE_LINEAR);
        inventory.items_textures["espada"] = tex;
        SDL_DestroySurface(sword_surf);
    }
}