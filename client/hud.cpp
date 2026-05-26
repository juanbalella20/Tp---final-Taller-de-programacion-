#include "hud.h"

HUD::HUD(SDL_Renderer* gui_renderer,
         float game_width, float panel_width, float canvas_height)
    : gui_renderer(gui_renderer),
      font(TTF_OpenFont("fonts/Roboto-VariableFont_wdth,wght.ttf", 16)),
      inventory{},
      attack_btn{game_width + 10.0f, canvas_height - 60.0f, panel_width - 20.0f, 40.0f},
      show_attack_button(false),
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
        auto it = inventory.items_textures.find(item.get_id());
        SDL_Texture* icon = (it != inventory.items_textures.end()) ? it->second : nullptr;

        if (icon) {
            SDL_FRect icon_dst = {slot_x, slot_y, SLOT_SIZE, SLOT_SIZE};
            SDL_RenderTexture(gui_renderer, icon, nullptr, &icon_dst);
        }

        slot_x += SLOT_SIZE + SLOT_MARGIN;
        if (slot_x + SLOT_SIZE > game_width + panel_width - SLOT_MARGIN) {
            slot_x = game_width + SLOT_MARGIN;
            slot_y += SLOT_SIZE + SLOT_MARGIN;
        }
    }
}

void HUD::load_textures() {
    SDL_Surface* inv_bg_surf = IMG_Load("imagenes/inventory-bg..png");
    if (inv_bg_surf) {
        inventory.bg_texture = SDL_CreateTextureFromSurface(gui_renderer, inv_bg_surf);
        SDL_DestroySurface(inv_bg_surf);
    }

    SDL_Surface* sword_surf = IMG_Load("imagenes/es_boton-espada-off.bmp");
    if (sword_surf) {
        SDL_SetSurfaceColorKey(sword_surf, true,
            SDL_MapRGB(SDL_GetPixelFormatDetails(sword_surf->format), nullptr, 0, 0, 0));
        inventory.items_textures["espada"] =
            SDL_CreateTextureFromSurface(gui_renderer, sword_surf);
        SDL_DestroySurface(sword_surf);
    }
}