#include "hud.h"
#include <stdexcept>

HUD::HUD(SDL_Renderer* gui_renderer,
         float game_width, float panel_width, float canvas_height)
    : gui_renderer(gui_renderer),
      font(TTF_OpenFont("fonts/StackSansText-Medium.ttf", 16)),
      inventory{},
      attack_btn{game_width + 10.0f, canvas_height - 60.0f, panel_width - 20.0f, 40.0f},
      show_attack_button(false),
      player_gold(0),
      player_hp(0),
      max_hp(100),
      max_xp(100),
      max_mana(100),
      player_xp(0),
      player_mana(0),
      hp_bar_texture(nullptr),
      xp_bar_texture(nullptr),
      mana_bar_texture(nullptr),
      gold_texture(nullptr),
      game_texture(nullptr),
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
    if (xp_bar_texture) {
        SDL_DestroyTexture(xp_bar_texture);
    }
    if (mana_bar_texture) {
        SDL_DestroyTexture(mana_bar_texture);
    }
    if (gold_texture) {
        SDL_DestroyTexture(gold_texture);
    }
    if (game_texture) {
        SDL_DestroyTexture(game_texture);
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

void HUD::set_max_mana(uint32_t max_mana_value) {
    max_mana = max_mana_value;
}

void HUD::set_xp(uint32_t xp) {
    //player_xp = xp;
    player_xp = 20;
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

void HUD::drawIconItem(const ItemInfo& item, float slot_x, float slot_y, float SLOT_SIZE) {
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
}

void HUD::drawItems() {
    const float SLOT_SIZE = 48.0f;
    const float SLOT_MARGIN = 8.0f;
    float slot_x = 785.0f;
    float slot_y = 210.0f;

    int slot_index = 0;
    for (const auto& item : inventory.items) {
        SDL_FRect slot_rect = {slot_x, slot_y, SLOT_SIZE, SLOT_SIZE};
        SDL_SetRenderDrawBlendMode(gui_renderer, SDL_BLENDMODE_BLEND);

        // Si este item esta equipado, dibujar un halo amarillo detrás
        if (equipped_slot >= 0 && slot_index == equipped_slot) {
            SDL_FRect halo = {slot_x - 3.0f, slot_y - 3.0f, SLOT_SIZE + 6.0f, SLOT_SIZE + 6.0f};
            SDL_SetRenderDrawColor(gui_renderer, 255, 215, 0, 120);
            SDL_RenderFillRect(gui_renderer, &halo);
        }

        SDL_SetRenderDrawColor(gui_renderer, 60, 60, 60, 120);
        SDL_RenderFillRect(gui_renderer, &slot_rect);
        SDL_SetRenderDrawColor(gui_renderer, 120, 120, 120, 180);
        SDL_RenderRect(gui_renderer, &slot_rect);

        drawIconItem(item, slot_x, slot_y, SLOT_SIZE);

        slot_x += SLOT_SIZE + SLOT_MARGIN;
        ++slot_index;
        if (slot_x + SLOT_SIZE > game_width + panel_width - SLOT_MARGIN) {
            slot_x = game_width + SLOT_MARGIN;
            slot_y += SLOT_SIZE + SLOT_MARGIN;
        }
    }
}

void HUD::drawInventoryItems() {

    // Items del inventario en slots
    drawItems();
}

void HUD::drawBigStat(SDL_Texture* tex, float pos_y, int current, int max) {
    float tex_w, tex_h;
    SDL_GetTextureSize(tex, &tex_w, &tex_h);

    float image_w = 1021.0f;
    float image_h = 767.0f;

    float scale_x = static_cast<float>(game_width + panel_width) / image_w;
    float scale_y = static_cast<float>(canvas_height) / image_h;

    float start_x = 788.0f;
    const float bar_w = 217.0f;
    const float bar_h = 18.0f;

    float x = start_x * scale_x;
    float y = pos_y * scale_y;
    float w = bar_w * scale_x;
    float h = bar_h * scale_y;

    float percentage = (max > 0) ? static_cast<float>(current) / static_cast<float>(max) : 0.0f;
    if (percentage < 0.0f) percentage = 0.0f;
    if (percentage > 1.0f) percentage = 1.0f;

    if (percentage > 0.0f) {
        SDL_FRect src_rect = { 0.0f, 0.0f, tex_w * percentage, tex_h };
        SDL_FRect fill_dest = { x, y, w * percentage, h };
        SDL_RenderTexture(gui_renderer, tex, &src_rect, &fill_dest);
    }

    SDL_FRect text_dest = { x, y, w, h };
    float text_scale = 0.75f;
    displayValue(current, max, text_dest, text_scale);
}

void HUD::drawSmallStat(SDL_Texture* tex, float pos_y, int current, int max) {
    float tex_w, tex_h;
    SDL_GetTextureSize(tex, &tex_w, &tex_h);

    float image_w = 1021.0f;
    float image_h = 767.0f;

    float scale_x = static_cast<float>(game_width + panel_width) / image_w;
    float scale_y = static_cast<float>(canvas_height) / image_h;

    float start_x = 788.0f;
    const float bar_w = 89.0f;
    const float bar_h = 12.0f;

    float x = start_x * scale_x;
    float y = pos_y * scale_y;
    float w = bar_w * scale_x;
    float h = bar_h * scale_y;

    float percentage = (max > 0) ? static_cast<float>(current) / static_cast<float>(max) : 0.0f;
    if (percentage < 0.0f) percentage = 0.0f;
    if (percentage > 1.0f) percentage = 1.0f;

    if (percentage > 0.0f) {
        SDL_FRect src_rect = { 0.0f, 0.0f, tex_w * percentage, tex_h };
        SDL_FRect fill_dest = { x, y, w * percentage, h };
        SDL_RenderTexture(gui_renderer, tex, &src_rect, &fill_dest);
    }

    SDL_FRect text_dest = { x, y, w, h };
    float text_scale = 0.55f;
    displayValue(current, max, text_dest, text_scale);
}

void HUD::drawText(const std::string& text, float x, float y, SDL_Color color) {
    SDL_Surface* surf = TTF_RenderText_Solid(font, text.c_str(), 0, color);
    if (surf) {
        SDL_Texture* tex = SDL_CreateTextureFromSurface(gui_renderer, surf);
        if (tex) {
            SDL_FRect dest = { 
                x,
                y, 
                static_cast<float>(surf->w) * 0.75f, 
                static_cast<float>(surf->h) * 0.75f
            };
            SDL_RenderTexture(gui_renderer, tex, nullptr, &dest);
            SDL_DestroyTexture(tex);
        }
        SDL_DestroySurface(surf);
    }
}

void HUD::drawGold() {
    if (!font) return;

    std::string gold_text = std::to_string(player_gold);
    SDL_Color gold_color = {255, 215, 0, 255};

    float text_start_x = 740.0f;
    float text_start_y = 418.0f;

    drawText(gold_text, text_start_x, text_start_y, gold_color);
}

void HUD::displayValue(int current, int max, SDL_FRect& dest, float text_scale) {
    if (font) {
        std::string text = std::to_string(current) + "/" + std::to_string(max);
        SDL_Color white = {255, 255, 255, 255};
        SDL_Surface* surf = TTF_RenderText_Solid(font, text.c_str(), 0, white);
        if (surf) {
            SDL_Texture* tex = SDL_CreateTextureFromSurface(gui_renderer, surf);
            if (tex) {
                //float text_scale = 0.75f;
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

void HUD::drawHp() {
    if (hp_bar_texture) {
        drawBigStat(hp_bar_texture, 600.0f, player_hp, max_hp);
    }
}

void HUD::drawXp() {
    if (xp_bar_texture) {
        drawSmallStat(xp_bar_texture, 658.0f, player_xp, max_xp);
    }
}

void HUD::drawMana() {
    if (mana_bar_texture) {
        drawBigStat(mana_bar_texture, 629.0f, player_mana, max_mana);
    }
}

void HUD::render() {
    if (game_texture) {
        SDL_FRect bg_dest = { 
            0.0f, 
            0.0f, 
            static_cast<float>(game_width + panel_width), 
            static_cast<float>(canvas_height)
        };
        SDL_RenderTexture(gui_renderer, game_texture, nullptr, &bg_dest);
    }

    drawInventoryItems();
    drawGold();
    drawHp();
    drawMana();
    drawXp();
}

void HUD::load_stat_texture(const std::string& path, SDL_Texture** texture) {
    SDL_Surface* bar_surf = IMG_Load(path.c_str());
    if (!bar_surf) {
        throw std::runtime_error(std::string("Failed to load texture: ") + path + " - " + SDL_GetError());
    }
    *texture = SDL_CreateTextureFromSurface(gui_renderer, bar_surf);
    SDL_DestroySurface(bar_surf);
    if (!*texture) {
        throw std::runtime_error(std::string("Failed to create texture from surface: ") + SDL_GetError());
    }
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

    load_stat_texture("imagenes/en_barradevida.bmp", &hp_bar_texture);
    load_stat_texture("imagenes/en_barraexperiencia.bmp", &xp_bar_texture);
    load_stat_texture("imagenes/en_barrademana.bmp", &mana_bar_texture);
    load_stat_texture("imagenes/100.png", &gold_texture);
    load_stat_texture("imagenes/en_ventanaprincipal.bmp", &game_texture);
}