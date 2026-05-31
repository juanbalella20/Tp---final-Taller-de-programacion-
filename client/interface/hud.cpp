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
      max_hp(100),
      max_xp(100),
      player_xp(0),
      player_mana(0),
      hp_bar_texture(nullptr),
      xp_bar_texture(nullptr),
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
    if (hp_bar_texture) {
        SDL_DestroyTexture(hp_bar_texture);
    }
    if (font) {
        TTF_CloseFont(font);
    }
}

void HUD::set_inventory(const std::vector<ItemInfo>& items) {
    inventory.items = items;
    // inventory changed -> clear optimistic selected slot
    equipped_slot = -1;
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

void HUD::set_max_hp(uint32_t max_hp_value) {
    max_hp = max_hp_value;
}

void HUD::set_xp(uint32_t xp) {
    player_xp = xp;
}

void HUD::set_mana(uint32_t mana) {
    player_mana = mana;
}

void HUD::set_equipped_item(const std::string& id) {
    equipped_item_id = id;
}

void HUD::set_equipped_slot(int slot_index) {
    equipped_slot = slot_index;
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

    int slot_index = 0;
    for (const auto& item : inventory.items) {
        SDL_FRect slot_rect = {slot_x, slot_y, SLOT_SIZE, SLOT_SIZE};
        SDL_SetRenderDrawBlendMode(gui_renderer, SDL_BLENDMODE_BLEND);

        // Si este item esta equipado, dibujar un halo amarillo detrás
        if (equipped_slot >= 0 && slot_index == equipped_slot) {
            SDL_FRect halo = {slot_x - 3.0f, slot_y - 3.0f, SLOT_SIZE + 6.0f, SLOT_SIZE + 6.0f};
            SDL_SetRenderDrawColor(gui_renderer, 255, 215, 0, 120); // amarillo semi
            SDL_RenderFillRect(gui_renderer, &halo);
        }

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
            
            SDL_FRect crop = {224.0f, 96.0f, 30.0f, 30.0f};;
            SDL_RenderTexture(gui_renderer, icon, &crop, &icon_dst);
        }

        slot_x += SLOT_SIZE + SLOT_MARGIN;
        ++slot_index;
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

void HUD::display_value(int current, int max, SDL_FRect& dest) {
    if (font) {
        std::string text = std::to_string(current) + "/" + std::to_string(max);
        SDL_Color white = {255, 255, 255, 255};
        SDL_Surface* surf = TTF_RenderText_Solid(font, text.c_str(), 0, white);
        if (surf) {
            SDL_Texture* tex = SDL_CreateTextureFromSurface(gui_renderer, surf);
            if (tex) {
                float text_scale = 0.75f;
                float scaled_w = surf->w * text_scale;
                float scaled_h = surf->h * text_scale;
                float text_x = dest.x + (dest.w - scaled_w) / 2.0f;
                float text_y = dest.y + (dest.h - scaled_h) / 2.0f;
                SDL_FRect text_dest = {text_x, text_y, scaled_w, scaled_h};
                SDL_RenderTexture(gui_renderer, tex, nullptr, &text_dest);
                SDL_DestroyTexture(tex);
            }
            SDL_DestroySurface(surf);
        }
    }
}

void HUD::draw_hp() {
    if (hp_bar_texture) {
        float tex_w, tex_h;
        SDL_GetTextureSize(hp_bar_texture, &tex_w, &tex_h);
        const float max_width = panel_width - 30.0f;
        const float scale = SDL_min(1.0f, max_width / tex_w);

        SDL_FRect dest = {
            game_width + 15.0f,
            430.0f,
            tex_w * scale,
            tex_h * scale
        };
        SDL_RenderTexture(gui_renderer, hp_bar_texture, nullptr, &dest);

        display_value(player_hp, max_hp, dest);
    }
}

void HUD::draw_xp() {
    if (xp_bar_texture) {
        float tex_w, tex_h;
        SDL_GetTextureSize(xp_bar_texture, &tex_w, &tex_h);
        const float max_width = panel_width - 30.0f;
        const float scale = SDL_min(1.0f, max_width / tex_w);

        SDL_FRect dest = {
            game_width + 15.0f,
            460.0f,
            tex_w * scale,
            tex_h * scale
        };
        SDL_RenderTexture(gui_renderer, xp_bar_texture, nullptr, &dest);

        display_value(player_xp, max_xp, dest);
    }
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

    SDL_Surface* sword_surf = IMG_Load("imagenes/101.png");
    if (sword_surf) {
        SDL_Texture* tex = SDL_CreateTextureFromSurface(gui_renderer, sword_surf);
        SDL_SetTextureBlendMode(tex, SDL_SCALEMODE_LINEAR);
        inventory.items_textures["espada"] = tex;
        SDL_DestroySurface(sword_surf);
    }

    SDL_Surface* hp_bar_surf = IMG_Load("imagenes/en_barradevida.bmp");
    if (!hp_bar_surf) {
        hp_bar_surf = IMG_Load("en_barradevida.bmp");
    }
    if (hp_bar_surf) {
        hp_bar_texture = SDL_CreateTextureFromSurface(gui_renderer, hp_bar_surf);
        SDL_DestroySurface(hp_bar_surf);
    }

    SDL_Surface* xp_bar_surf = IMG_Load("imagenes/en_barraexperiencia.bmp");
    if (!xp_bar_surf) {
        xp_bar_surf = IMG_Load("en_barraexperiencia.bmp");
    }
    if (xp_bar_surf) {
        xp_bar_texture = SDL_CreateTextureFromSurface(gui_renderer, xp_bar_surf);
        SDL_DestroySurface(xp_bar_surf);
    }

}