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

    // Cache de una textura de texto: evita re-rasterizar la fuente (caro:
    // TTF_RenderText + subir surface a GPU) cada frame. Solo se regenera
    // cuando el string o el color cambian respecto al frame anterior.
    struct TextCache {
        SDL_Texture* texture = nullptr;
        std::string last_text;
        SDL_Color last_color = {0, 0, 0, 0};
        float w = 0.0f;
        float h = 0.0f;
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
    int player_level;
    SDL_Texture* hp_bar_texture;
    SDL_Texture* xp_bar_texture;
    SDL_Texture* mana_bar_texture;
    SDL_Texture* gold_texture;
    SDL_Texture* game_texture;
    // Fondo oscuro que tapa los íconos de stats de la parte inferior del panel y
    // sirve de fondo al panel de estado de equipo (escudo/armadura/casco).
    SDL_Texture* info_area_texture;
    // Slots actualmente equipados (índices en inventory.items). Hay como máximo
    // uno por tipo de item (arma, armadura, casco, escudo). DERIVADO de
    // equipped_uids: se recalcula cada vez que cambia el inventario.
    std::set<int> equipped_slots;
    // Uids de INSTANCIA equipados (estado real confirmado por el server). Es la
    // fuente de verdad del resaltado: equipped_slots se deriva de acá. Persiste
    // entre cambios de inventario (tomar/soltar items) para que el halo amarillo
    // no se pierda al reordenarse o agregarse items al inventario.
    std::vector<std::string> equipped_uids;
    std::string equipped_item_id;

    // Texturas de texto cacheadas (una por cada valor que se dibuja como texto).
    TextCache gold_text_cache;
    TextCache hp_text_cache;
    TextCache mana_text_cache;
    TextCache xp_text_cache;
    TextCache name_text_cache;
    TextCache level_text_cache;
    // Una por fila del panel de equipo (escudo, armadura, casco) para no
    // re-rasterizar el texto EQUIPED/NOT EQUIPED cada frame.
    TextCache equip_status_caches[3];

    float game_width;
    float panel_width;
    float canvas_height;
    float image_w;
    float image_h;
    float scale_x;
    float scale_y;

    void load_textures();
    void load_stat_texture(const std::string& path, SDL_Texture** texture);
    void drawBigStat(SDL_Texture* tex, float pos_y, int current, int max, TextCache& cache);
    void drawSmallStat(SDL_Texture* tex, float pos_y, int current, int max, TextCache& cache);
    void displayValue(int current, int max, SDL_FRect& dest, float text_scale, TextCache& cache);
    void drawText(const std::string& text, float x, float y, SDL_Color color, TextCache& cache);
    // Regenera la textura del cache solo si el texto/color cambiaron; si no, la reusa.
    void update_text_cache(TextCache& cache, const std::string& text, SDL_Color color);
    void drawItems();
    void drawIconItem(const ItemInfo& item, float slot_x, float slot_y, float SLOT_SIZE);
    // Recalcula equipped_slots (índices a resaltar) a partir de equipped_uids y el
    // inventario actual. Se llama cada vez que cambia el inventario o el equipo.
    void refresh_equipped_slots();
    // Panel inferior con el estado de equipo de las defensas (escudo/armadura/
    // casco): por cada una que esté en el inventario, su ícono + EQUIPED/NOT.
    void drawEquipStatus();
    // true si el item del slot dado está equipado (su uid figura en equipped_uids).
    bool is_slot_equipped(int slot_index) const;

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
    void set_max_xp(uint32_t max_xp);
    void set_xp(uint32_t xp);
    void set_mana(uint32_t mana);
    void set_equipped_item(const std::string& item_id);
    void set_level(int level);
    // Equipa el slot dado: des-resalta cualquier otro slot del mismo tipo y
    // resalta este. Permite tener equipados varios items de distinto tipo.
    void set_equipped_slot(int slot_index);
    // Alterna el resaltado de un slot: si estaba equipado lo desequipa, si no lo equipa.
    void toggle_equipped_slot(int slot_index);
    // Resalta exactamente los slots cuyos items tengan alguno de estos uids de
    // INSTANCIA (estado real de equipo confirmado por el server). Por uid, no por
    // tipo: dos copias del mismo item no se resaltan juntas.
    void set_equipped_by_uids(const std::vector<std::string>& uids);
    void drawInventoryItems();
    void drawAttackButton();
    void drawHp();
    void drawMana();
    void drawGold();
    void drawXp();
    void display_player_info(const std::string& player_name);
    void render();
};

#endif