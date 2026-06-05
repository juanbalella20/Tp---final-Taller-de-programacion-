#include "hud.h"
#include <stdexcept>

HUD::HUD(SDL_Renderer* gui_renderer,
         float game_width, float panel_width, float canvas_height)
    : gui_renderer(gui_renderer),
      font(TTF_OpenFont("fonts/StackSansText-Medium.ttf", 16)),
      inventory{},
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
    
    load_textures();
}

HUD::~HUD() {
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
    if (gold_text_cache.texture) {
        SDL_DestroyTexture(gold_text_cache.texture);
    }
    if (hp_text_cache.texture) {
        SDL_DestroyTexture(hp_text_cache.texture);
    }
    if (mana_text_cache.texture) {
        SDL_DestroyTexture(mana_text_cache.texture);
    }
    if (xp_text_cache.texture) {
        SDL_DestroyTexture(xp_text_cache.texture);
    }

    if (font) {
        TTF_CloseFont(font);
    }
}

void HUD::set_inventory(const std::vector<ItemInfo>& items) {
    inventory.items = items;
    // inventory cambió -> limpiar resaltados optimistas
    equipped_slots.clear();
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
    if (slot_index < 0 || slot_index >= static_cast<int>(inventory.items.size())) {
        return;
    }
    uint8_t type = inventory.items[slot_index].get_type();

    // Des-resalta cualquier slot ya equipado del mismo tipo (uno por tipo).
    for (auto it = equipped_slots.begin(); it != equipped_slots.end();) {
        if (*it < static_cast<int>(inventory.items.size()) &&
            inventory.items[*it].get_type() == type) {
            it = equipped_slots.erase(it);
        } else {
            ++it;
        }
    }
    equipped_slots.insert(slot_index);
}

void HUD::set_equipped_by_ids(const std::vector<std::string>& ids) {
    equipped_slots.clear();
    for (int i = 0; i < static_cast<int>(inventory.items.size()); ++i) {
        const std::string& item_id = inventory.items[i].get_id();
        for (const std::string& id : ids) {
            if (item_id == id) {
                equipped_slots.insert(i);
                break;
            }
        }
    }
}

void HUD::toggle_equipped_slot(int slot_index) {
    if (slot_index < 0 || slot_index >= static_cast<int>(inventory.items.size())) {
        return;
    }
    // Si ya estaba equipado, lo desequipa (se quita el halo amarillo).
    if (equipped_slots.count(slot_index) > 0) {
        equipped_slots.erase(slot_index);
        return;
    }
    // Si no, lo equipa (reemplazando cualquier otro del mismo tipo).
    set_equipped_slot(slot_index);
}

void HUD::drawIconItem(const ItemInfo& item, float slot_x, float slot_y, float SLOT_SIZE) {
    auto it = inventory.items_textures.find(item.get_id());
    if (it != inventory.items_textures.end() && it->second != nullptr) {
        SDL_Texture* icon = it->second;

        // Crop específico del item; fallback al de la espada si no está registrado.
        SDL_FRect crop = {224.0f, 96.0f, 30.0f, 30.0f};
        auto crop_it = inventory.items_crops.find(item.get_id());
        if (crop_it != inventory.items_crops.end()) {
            crop = crop_it->second;
        }

        // Escala según el tamaño del sprite recortado, no de todo el spritesheet.
        const float PADDING = 4.0f;
        float max_size = SLOT_SIZE - (PADDING * 2.0f);
        float scale = SDL_min(max_size / crop.w, max_size / crop.h);
        float final_w = crop.w * scale;
        float final_h = crop.h * scale;

        SDL_FRect icon_dst = {
            slot_x + (SLOT_SIZE - final_w) / 2.0f,
            slot_y + (SLOT_SIZE - final_h) / 2.0f,
            final_w,
            final_h
        };

        SDL_RenderTexture(gui_renderer, icon, &crop, &icon_dst);
    }
}

void HUD::drawItems() {
    float image_w = 1021.0f;
    float image_h = 767.0f;

    float scale_x = static_cast<float>(game_width + panel_width) / image_w;
    float scale_y = static_cast<float>(canvas_height) / image_h;

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
    for (const auto& item : inventory.items) {
        SDL_FRect slot_rect = {slot_x, slot_y, slot_size, slot_size};
        SDL_SetRenderDrawBlendMode(gui_renderer, SDL_BLENDMODE_BLEND);

        // Si este item esta equipado, dibujar un halo amarillo detrás
        if (equipped_slots.count(slot_index) > 0) {
            SDL_FRect halo = {slot_x - 3.0f, slot_y - 3.0f, slot_size + 6.0f, slot_size + 6.0f};
            SDL_SetRenderDrawColor(gui_renderer, 255, 215, 0, 120);
            SDL_RenderFillRect(gui_renderer, &halo);
        }

        SDL_SetRenderDrawColor(gui_renderer, 60, 60, 60, 120);
        SDL_RenderFillRect(gui_renderer, &slot_rect);
        SDL_SetRenderDrawColor(gui_renderer, 120, 120, 120, 180);
        SDL_RenderRect(gui_renderer, &slot_rect);

        drawIconItem(item, slot_x, slot_y, slot_size);

        slot_x += slot_size + margin_x;
        ++slot_index;
        if (slot_x + slot_size > limit_x) {
            slot_x = start_x;
            slot_y += slot_size + margin_y;
        }
    }
}

void HUD::drawInventoryItems() {

    // Items del inventario en slots
    drawItems();
}

void HUD::drawBigStat(SDL_Texture* tex, float pos_y, int current, int max, TextCache& cache) {
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
    displayValue(current, max, text_dest, text_scale, cache);
}

void HUD::drawSmallStat(SDL_Texture* tex, float pos_y, int current, int max, TextCache& cache) {
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
    displayValue(current, max, text_dest, text_scale, cache);
}

void HUD::update_text_cache(TextCache& cache, const std::string& text, SDL_Color color) {
    // Reusar si nada cambió: lo común frame a frame.
    bool same_color = cache.last_color.r == color.r && cache.last_color.g == color.g &&
                      cache.last_color.b == color.b && cache.last_color.a == color.a;
    if (cache.texture && cache.last_text == text && same_color) {
        return;
    }

    if (cache.texture) {
        SDL_DestroyTexture(cache.texture);
        cache.texture = nullptr;
    }
    cache.w = 0.0f;
    cache.h = 0.0f;

    SDL_Surface* surf = TTF_RenderText_Solid(font, text.c_str(), 0, color);
    if (surf) {
        cache.texture = SDL_CreateTextureFromSurface(gui_renderer, surf);
        if (cache.texture) {
            cache.w = static_cast<float>(surf->w);
            cache.h = static_cast<float>(surf->h);
        }
        SDL_DestroySurface(surf);
    }

    cache.last_text = text;
    cache.last_color = color;
}

void HUD::drawText(const std::string& text, float x, float y, SDL_Color color, TextCache& cache) {
    update_text_cache(cache, text, color);
    if (cache.texture) {
        SDL_FRect dest = { x, y, cache.w * 0.70f, cache.h * 0.70f };
        SDL_RenderTexture(gui_renderer, cache.texture, nullptr, &dest);
    }
}

void HUD::drawGold() {
    if (!font) return;

    std::string gold_text = std::to_string(player_gold);
    SDL_Color gold_color = {255, 215, 0, 255};

    float image_w = 1021.0f;
    float image_h = 767.0f;

    float scale_x = static_cast<float>(game_width + panel_width) / image_w;
    float scale_y = static_cast<float>(canvas_height) / image_h;

    float text_start_x = 788.0f * scale_x;
    float text_start_y = 558.0f * scale_y;

    drawText(gold_text, text_start_x, text_start_y, gold_color, gold_text_cache);
}

void HUD::displayValue(int current, int max, SDL_FRect& dest, float text_scale, TextCache& cache) {
    if (font) {
        std::string text = std::to_string(current) + "/" + std::to_string(max);
        SDL_Color white = {255, 255, 255, 255};
        update_text_cache(cache, text, white);
        if (cache.texture) {
            float scaled_w = cache.w * text_scale;
            float scaled_h = cache.h * text_scale;
            float text_x = dest.x + (dest.w - scaled_w) / 2.0f;
            float text_y = dest.y + (dest.h - scaled_h) / 2.0f;
            SDL_FRect text_dest = {text_x, text_y, scaled_w, scaled_h};
            SDL_RenderTexture(gui_renderer, cache.texture, nullptr, &text_dest);
        }
    }
}

void HUD::drawHp() {
    if (hp_bar_texture) {
        drawBigStat(hp_bar_texture, 600.0f, player_hp, max_hp, hp_text_cache);
    }
}

void HUD::drawXp() {
    if (xp_bar_texture) {
        drawSmallStat(xp_bar_texture, 658.0f, player_xp, max_xp, xp_text_cache);
    }
}

void HUD::drawMana() {
    if (mana_bar_texture) {
        drawBigStat(mana_bar_texture, 629.0f, player_mana, max_mana, mana_text_cache);
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
    SDL_Surface* sword_surf = IMG_Load("imagenes/101.png");
    if (sword_surf) {
        SDL_Texture* tex = SDL_CreateTextureFromSurface(gui_renderer, sword_surf);
        SDL_SetTextureBlendMode(tex, SDL_SCALEMODE_LINEAR);
        inventory.items_textures["espada"] = tex;
        inventory.items_crops["espada"] = {224.0f, 96.0f, 30.0f, 30.0f};
        SDL_DestroySurface(sword_surf);
    }

    SDL_Surface* shield_surf = IMG_Load("imagenes/2141.png");
    if (shield_surf) {
        SDL_Texture* tex = SDL_CreateTextureFromSurface(gui_renderer, shield_surf);
        SDL_SetTextureBlendMode(tex, SDL_SCALEMODE_LINEAR);
        inventory.items_textures["escudo"] = tex;
        // Primer sprite del escudo (esquina superior izquierda del spritesheet).
        inventory.items_crops["escudo"] = {0.0f, 0.0f, 32.0f, 32.0f};
        SDL_DestroySurface(shield_surf);
    }

    load_stat_texture("imagenes/en_barradevida.bmp", &hp_bar_texture);
    load_stat_texture("imagenes/en_barraexperiencia.bmp", &xp_bar_texture);
    load_stat_texture("imagenes/en_barrademana.bmp", &mana_bar_texture);
    load_stat_texture("imagenes/100.png", &gold_texture);
    load_stat_texture("imagenes/en_ventanaprincipal.png", &game_texture);
}