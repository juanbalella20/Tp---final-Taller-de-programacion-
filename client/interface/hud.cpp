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
      player_level(1),
      hp_bar_texture(nullptr),
      xp_bar_texture(nullptr),
      mana_bar_texture(nullptr),
      gold_texture(nullptr),
      game_texture(nullptr),
      info_area_texture(nullptr),
      game_width(game_width),
      panel_width(panel_width),
      canvas_height(canvas_height) {
    
    image_w = 1021.0f;
    image_h = 767.0f;

    scale_x = static_cast<float>(game_width + panel_width) / image_w;
    scale_y = static_cast<float>(canvas_height) / image_h;

    load_textures();
}

HUD::~HUD() {
    // Varios ids pueden compartir la misma textura (p.ej. ambas pociones salen
    // del mismo 100.png): destruir cada textura UNA sola vez para no doble-liberar.
    std::set<SDL_Texture*> destroyed;
    for (auto& kv : inventory.items_textures) {
        if (kv.second && destroyed.insert(kv.second).second) {
            SDL_DestroyTexture(kv.second);
        }
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
    if (info_area_texture) {
        SDL_DestroyTexture(info_area_texture);
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
    if (name_text_cache.texture) {
        SDL_DestroyTexture(name_text_cache.texture);
    }
    if (level_text_cache.texture) {
        SDL_DestroyTexture(level_text_cache.texture);
    }
    for (auto& cache : equip_status_caches) {
        if (cache.texture) SDL_DestroyTexture(cache.texture);
    }

    if (font) {
        TTF_CloseFont(font);
    }
}

void HUD::set_inventory(const std::vector<ItemInfo>& items) {
    inventory.items = items;
    // El inventario cambió (p.ej. se tomó/soltó un item): los índices viejos ya
    // no sirven, pero el equipo real no cambió. Re-derivamos el resaltado a
    // partir de los uids equipados (fuente de verdad) para que el halo amarillo
    // sobreviva al reordenamiento.
    refresh_equipped_slots();
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

void HUD::set_max_xp(uint32_t max_xp_value) {
    max_xp = max_xp_value;
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

void HUD::set_level(int level) {
    player_level = level;
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

void HUD::set_equipped_by_uids(const std::vector<std::string>& uids) {
    // Resalta por uid de INSTANCIA: dos copias del mismo tipo tienen distinto
    // uid, así que solo se resalta la que realmente está equipada. Guardamos los
    // uids como fuente de verdad para poder re-derivar el resaltado cuando cambie
    // el inventario (ver set_inventory / refresh_equipped_slots).
    equipped_uids = uids;
    refresh_equipped_slots();
}

void HUD::refresh_equipped_slots() {
    // Recalcula qué slots resaltar a partir de equipped_uids y el inventario
    // actual. Es seguro llamarla cada vez que cambia cualquiera de los dos.
    equipped_slots.clear();
    for (int i = 0; i < static_cast<int>(inventory.items.size()); ++i) {
        if (is_slot_equipped(i)) equipped_slots.insert(i);
    }
}

bool HUD::is_slot_equipped(int slot_index) const {
    if (slot_index < 0 || slot_index >= static_cast<int>(inventory.items.size())) {
        return false;
    }
    const std::string item_uid = std::to_string(inventory.items[slot_index].get_uid());
    for (const std::string& uid : equipped_uids) {
        if (item_uid == uid) return true;
    }
    return false;
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
    // Textura del item por id; si no está registrada, cae a la de la espada para
    // que el slot nunca quede vacío (un item siempre se ve con ALGÚN sprite).
    SDL_Texture* icon = nullptr;
    auto it = inventory.items_textures.find(item.get_id());
    if (it != inventory.items_textures.end() && it->second != nullptr) {
        icon = it->second;
    } else {
        auto fallback = inventory.items_textures.find("espada");
        if (fallback != inventory.items_textures.end()) icon = fallback->second;
    }
    if (icon != nullptr) {
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

void HUD::drawXpBar(SDL_Texture* tex, int current, int max, TextCache& cache) {
    float tex_w, tex_h;
    SDL_GetTextureSize(tex, &tex_w, &tex_h);

    float start_x = 767.0f;
    float pos_y = 99.0f;
    const float bar_w = 243.0f;
    const float bar_h = 23.0f;

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
    float text_scale = 0.60f;
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
        drawXpBar(xp_bar_texture, player_xp, max_xp, xp_text_cache);
    }
}

void HUD::drawMana() {
    if (mana_bar_texture) {
        drawBigStat(mana_bar_texture, 629.0f, player_mana, max_mana, mana_text_cache);
    }
}

void HUD::drawEquipStatus() {
    // Panel inferior: estado de equipo de las defensas. Tapa los íconos de stats
    // que trae la imagen de fondo del HUD con info-area-center y encima dibuja,
    // por cada tipo (escudo, armadura, casco) que el jugador TENGA en el
    // inventario: el ícono del item + "<Tipo> Equipado" (verde) / "<Tipo> No
    // equiapado" (rojo). Los tipos que no se tienen no aparecen. Coordenadas en el
    // espacio de la imagen base (1021x767), igual que el resto del HUD.
    if (!font) return;

    // Zona del contenido (coords base) y fondo ajustado al ancho real del panel.
    // El fondo deja visible únicamente el borde decorativo lateral del HUD.
    const float content_x = 775.0f;
    const float area_y = 672.0f;
    const float area_h = 92.0f;
    const float panel_border_inset = 8.0f;

    if (info_area_texture) {
        SDL_FRect area_dst = {
            game_width + panel_border_inset,
            area_y * scale_y,
            panel_width - panel_border_inset * 2.0f,
            area_h * scale_y
        };
        SDL_RenderTexture(gui_renderer, info_area_texture, nullptr, &area_dst);
    }

    // Tipos a mostrar, en orden, con su etiqueta. (1=armadura, 2=casco,
    // 3=escudo en ItemInfo).
    struct Row { uint8_t type; const char* label; };
    const Row rows[] = {
        {3, "Escudo"},
        {1, "Armadura"},
        {2, "Casco"},
    };

    // Geometría de las filas dentro de la zona cubierta.
    const float row_h   = 26.0f;
    const float first_y = area_y + 6.0f;
    const float icon_sz = 20.0f;
    const float icon_pad = 8.0f;

    int drawn = 0;  // filas efectivamente dibujadas (para apilarlas sin huecos)
    int cache_idx = 0;
    for (const Row& row : rows) {
        // Buscar el primer item de este tipo en el inventario; saber si alguno de
        // ese tipo está equipado.
        const ItemInfo* item = nullptr;
        bool equipped = false;
        for (int i = 0; i < static_cast<int>(inventory.items.size()); ++i) {
            if (inventory.items[i].get_type() != row.type) continue;
            if (item == nullptr) item = &inventory.items[i];
            if (is_slot_equipped(i)) { equipped = true; break; }
        }
        if (item == nullptr) continue;  // no se tiene ese tipo: fila oculta

        float row_y = first_y + drawn * row_h;
        ++drawn;

        // Ícono del item (reutiliza el mapeo id->textura/crop del inventario).
        float icon_slot = icon_sz * scale_x;
        float icon_x = (content_x + icon_pad) * scale_x;
        float icon_y = (row_y + (row_h - icon_sz) / 2.0f) * scale_y;
        drawIconItem(*item, icon_x, icon_y, icon_slot);

        // Texto "<Tipo> Equipado / No equipado" a la derecha del ícono. El color
        // marca el estado (verde = equipado, rojo = no).
        std::string text = std::string(row.label) +
                           (equipped ? " Equipado" : " No equipado");
        SDL_Color color = equipped ? SDL_Color{40, 200, 60, 255}
                                   : SDL_Color{210, 50, 50, 255};
        TextCache& cache = equip_status_caches[cache_idx++];
        update_text_cache(cache, text, color);
        if (cache.texture) {
            float text_scale = 0.60f;
            float tw = cache.w * text_scale;
            float th = cache.h * text_scale;
            float text_x = (content_x + icon_pad * 2.0f + icon_sz) * scale_x;
            float text_y = (row_y * scale_y) + (row_h * scale_y - th) / 2.0f;
            SDL_FRect dest = { text_x, text_y, tw, th };
            SDL_RenderTexture(gui_renderer, cache.texture, nullptr, &dest);
        }
    }
}

void HUD::display_player_info(const std::string& player_name) {
    if (!font) return;
    
    SDL_Color color = {255, 255, 255, 255};

    float center_x = 878.0f * scale_x;
    float text_start_y = 46.0f * scale_y;

    update_text_cache(name_text_cache, player_name, color);
    if (name_text_cache.texture) {
        float scale = 0.90f;
        float text_width = name_text_cache.w * scale;
        float text_start_x = center_x - (text_width / 2.0f);
        SDL_FRect dest = { text_start_x, text_start_y, text_width, name_text_cache.h * scale };
        SDL_RenderTexture(gui_renderer, name_text_cache.texture, nullptr, &dest);
    }

    std::string level_text = "Level: " + std::to_string(player_level);
    text_start_y = 75.0f * scale_y;

    update_text_cache(level_text_cache, level_text, color);
    if (level_text_cache.texture) {
        float scale = 0.70f;
        float text_width = level_text_cache.w * scale;
        float text_start_x = center_x - (text_width / 2.0f);
        SDL_FRect dest = { text_start_x, text_start_y, text_width, level_text_cache.h * scale };
        SDL_RenderTexture(gui_renderer, level_text_cache.texture, nullptr, &dest);
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
    drawEquipStatus();
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

    // El sprite 2141.png es el "escudo de hierro" (la imagen que se mostraba
    // antes bajo el alias "escudo"). El escudo de tortuga usa su propio PNG.
    SDL_Surface* shield_surf = IMG_Load("imagenes/2141.png");
    if (shield_surf) {
        SDL_Texture* tex = SDL_CreateTextureFromSurface(gui_renderer, shield_surf);
        SDL_SetTextureBlendMode(tex, SDL_SCALEMODE_LINEAR);
        inventory.items_textures["escudo_hierro"] = tex;
        // Primer sprite del escudo (esquina superior izquierda del spritesheet).
        inventory.items_crops["escudo_hierro"] = {0.0f, 0.0f, 32.0f, 32.0f};
        SDL_DestroySurface(shield_surf);
    }

    // Resto de los items: cada PNG es un único sprite que ocupa toda la imagen
    // (1024x1024), así que el crop cubre la imagen entera y drawIconItem la
    // escala al slot. La clave es el id del item (igual que el server). El
    // escudo de tortuga se asigna también al alias histórico "escudo" (misma
    // textura: el destructor deduplica, así que no hay doble-free).
    struct ItemIcon { const char* id; const char* path; };
    const ItemIcon item_icons[] = {
        // Varas/báculos.
        {"vara_fresno",      "imagenes/icon_vara_fresno.png"},
        {"baculo_nudoso",    "imagenes/icon_baculo_nudoso.png"},
        {"baculo_engarzado", "imagenes/icon_baculo_engarzado.png"},
        {"flauta_elfica",    "imagenes/icon_flauta_elfica.png"},
        // Armas físicas.
        {"hacha",            "imagenes/hacha.png"},
        {"martillo",         "imagenes/martillo.png"},
        {"arco_simple",      "imagenes/arco-simple.png"},
        {"arco_compuesto",   "imagenes/arco-compuesto.png"},
        // Armaduras.
        {"armadura_cuero",   "imagenes/Armadura-de-cuero.png"},
        {"armadura_placas",  "imagenes/armadura-de-placas.png"},
        {"tunica_azul",      "imagenes/tunica-azul.png"},
        // Cascos.
        {"capucha",          "imagenes/capucha.png"},
        {"casco_hierro",     "imagenes/casco-de-hierro.png"},
        {"sombrero_magico",  "imagenes/sombrero-magico.png"},
        // Escudos.
        {"escudo_tortuga",   "imagenes/escudo-tortuga.png"},
    };
    for (const auto& ii : item_icons) {
        SDL_Surface* surf = IMG_Load(ii.path);
        if (!surf) continue;
        float w = static_cast<float>(surf->w);
        float h = static_cast<float>(surf->h);
        SDL_Texture* tex = SDL_CreateTextureFromSurface(gui_renderer, surf);
        SDL_SetTextureBlendMode(tex, SDL_SCALEMODE_LINEAR);
        SDL_DestroySurface(surf);
        if (!tex) continue;
        inventory.items_textures[ii.id] = tex;
        inventory.items_crops[ii.id] = {0.0f, 0.0f, w, h};
    }

    // Alias histórico "escudo" -> escudo de tortuga (comparte textura/crop).
    auto tortuga_tex = inventory.items_textures.find("escudo_tortuga");
    if (tortuga_tex != inventory.items_textures.end()) {
        inventory.items_textures["escudo"] = tortuga_tex->second;
        inventory.items_crops["escudo"] = inventory.items_crops["escudo_tortuga"];
    }

    // Pociones en el inventario: ambas se recortan del MISMO spritesheet 100.png.
    // Se carga una sola textura y los dos ids la comparten (el destructor libera
    // cada textura una sola vez). pocion_vida = botella gris; pocion_mana = azul.
    SDL_Surface* potion_surf = IMG_Load("imagenes/100.png");
    if (potion_surf) {
        SDL_Texture* potion_tex = SDL_CreateTextureFromSurface(gui_renderer, potion_surf);
        SDL_DestroySurface(potion_surf);
        if (potion_tex) {
            SDL_SetTextureBlendMode(potion_tex, SDL_BLENDMODE_BLEND);
            inventory.items_textures["pocion_vida"] = potion_tex;
            inventory.items_crops["pocion_vida"] = {392.0f, 159.0f, 16.0f, 26.0f};
            inventory.items_textures["pocion_mana"] = potion_tex;
            inventory.items_crops["pocion_mana"] = {424.0f, 159.0f, 17.0f, 26.0f};
        }
    }

    load_stat_texture("imagenes/en_barradevida.bmp", &hp_bar_texture);
    load_stat_texture("imagenes/en_barraexperiencia.bmp", &xp_bar_texture);
    load_stat_texture("imagenes/en_barrademana.bmp", &mana_bar_texture);
    load_stat_texture("imagenes/100.png", &gold_texture);
    load_stat_texture("imagenes/en_ventanaprincipal.png", &game_texture);
    // Fondo del panel de estado de equipo (el archivo se llama con doble punto).
    load_stat_texture("imagenes/info-area-center..png", &info_area_texture);
}
