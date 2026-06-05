#ifndef HUD_H
#define HUD_H

#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3_ttf/SDL_ttf.h>
#include "item_info.h"
#include <vector>
#include <map>
#include <set>
#include <string>

class HUD {
private:
    struct InventoryView {
        std::vector<ItemInfo> items;
        std::map<std::string, SDL_Texture*> items_textures;
        // Recorte del sprite dentro del spritesheet de cada item (por id).
        std::map<std::string, SDL_FRect> items_crops;
    };

    SDL_Renderer* gui_renderer;
    TTF_Font* font;
    InventoryView inventory;
    uint32_t player_gold;
    uint32_t player_hp;
    uint32_t max_hp;
    uint32_t max_xp;
    uint32_t max_mana;
    uint32_t player_xp;
    uint32_t player_mana;
    SDL_Texture* hp_bar_texture;
    SDL_Texture* xp_bar_texture;
    SDL_Texture* mana_bar_texture;
    SDL_Texture* gold_texture;
    SDL_Texture* game_texture;
    // Slots actualmente equipados (índices en inventory.items). Hay como máximo
    // uno por tipo de item (arma, armadura, casco, escudo).
    std::set<int> equipped_slots;
    std::string equipped_item_id;

    float game_width;
    float panel_width;
    float canvas_height;

    void load_textures();
    void load_stat_texture(const std::string& path, SDL_Texture** texture);
    void drawBigStat(SDL_Texture* tex, float pos_y, int current, int max);
    void drawSmallStat(SDL_Texture* tex, float pos_y, int current, int max);
    void displayValue(int current, int max, SDL_FRect& dest, float text_scale);
    void drawText(const std::string& text, float x, float y, SDL_Color color);
    void drawItems();
    void drawIconItem(const ItemInfo& item, float slot_x, float slot_y, float SLOT_SIZE);

public:
    HUD(SDL_Renderer* gui_renderer,
        float game_width, float panel_width, float canvas_height);
    ~HUD();

    HUD(const HUD&) = delete;
    HUD& operator=(const HUD&) = delete;

    void set_inventory(const std::vector<ItemInfo>& items);
    const std::vector<ItemInfo>& get_inventory() const { return inventory.items; }
    void set_gold(uint32_t amount);
    void set_hp(uint32_t hp);
    void set_max_hp(uint32_t max_hp);
    void set_max_mana(uint32_t max_mana);
    void set_xp(uint32_t xp);
    void set_mana(uint32_t mana);
    void set_equipped_item(const std::string& item_id);
    // Equipa el slot dado: des-resalta cualquier otro slot del mismo tipo y
    // resalta este. Permite tener equipados varios items de distinto tipo.
    void set_equipped_slot(int slot_index);
    // Alterna el resaltado de un slot: si estaba equipado lo desequipa, si no lo equipa.
    void toggle_equipped_slot(int slot_index);
    // Resalta exactamente los slots cuyos items tengan alguno de estos ids
    // (estado real de equipo confirmado por el server).
    void set_equipped_by_ids(const std::vector<std::string>& ids);
    void drawInventoryItems();
    void drawAttackButton();
    void drawHp();
    void drawMana();
    void drawGold();
    void drawXp();
    void render();
};

#endif