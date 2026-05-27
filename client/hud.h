#ifndef HUD_H
#define HUD_H

#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3_ttf/SDL_ttf.h>
#include "../common/item_info.h"
#include <vector>
#include <map>
#include <string>

class HUD {
private:
    struct InventoryView {
        SDL_FRect panel;
        SDL_Texture* bg_texture;
        std::vector<ItemInfo> items;
        std::map<std::string, SDL_Texture*> items_textures;
    };

    SDL_Renderer* gui_renderer;
    TTF_Font* font;
    InventoryView inventory;
    SDL_FRect attack_btn;
    bool show_attack_button;

    uint32_t gold_amount;

    float game_width;
    float panel_width;
    float canvas_height;

    void load_textures();

public:
    HUD(SDL_Renderer* gui_renderer,
        float game_width, float panel_width, float canvas_height);
    ~HUD();

    HUD(const HUD&) = delete;
    HUD& operator=(const HUD&) = delete;

    void set_inventory(const std::vector<ItemInfo>& items);
    const std::vector<ItemInfo>& get_inventory() const { return inventory.items; }
    void set_attack_button_visible(bool visible);
    bool is_attack_button_visible() const { return show_attack_button; }
    const SDL_FRect& get_attack_button_rect() const { return attack_btn; }

    void set_gold(uint32_t amount);

    void drawInventoryPanel();
    void drawAttackButton();
    //TODO:
    void draw_hp();
    void draw_mana();
    void draw_gold();
    void draw_xp();
};

#endif