#ifndef TEXTURE_LOADER_H_
#define TEXTURE_LOADER_H_

#include <SDL3/SDL.h>
#include <string>
#include <map>
#include "paths.h"

class TextureLoader {

private:
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Texture* player_texture;
    SDL_Texture* gold_texture;
    SDL_Texture* ghost_texture;
    SDL_Texture* head_texture;
    SDL_Texture* hat_texture;
    SDL_Texture* seller_texture;
    SDL_Texture* banker_texture;
    SDL_Texture* priest_texture;
    SDL_Texture* hp_bar_texture;
    SDL_Texture* xp_bar_texture;
    SDL_Texture* mana_bar_texture;
    SDL_Texture* game_texture;
    SDL_Texture* shop_texture;

    std::map<std::string, SDL_Texture*> enemies_textures;
    std::map<std::string, SDL_Texture*> spell_effects_textures;
    std::map<std::string, SDL_Texture*> item_textures;

    void load_npc_enemy(const std::string& npc_name, const std::string& image_path);

    void load_specific(const std::string& image_path, SDL_Texture** texture);

    void free_texture(SDL_Texture** texture);

public:
    TextureLoader(SDL_Window* window, SDL_Renderer* renderer);
    TextureLoader(SDL_Renderer* renderer);

    void load_game_logo();

    void load_npcs_enemies();

    SDL_Texture* get_texture_of_npc_enemy(const std::string& npc_name);

    SDL_Texture* get_texture_of_spell(const std::string& spell_id);

    SDL_Texture* get_texture_of_item(const std::string& item);

    SDL_Texture* get_gold_texture();

    SDL_Texture* get_seller_texture();

    SDL_Texture* get_banker_texture();

    SDL_Texture* get_priest_texture();

    SDL_Texture* get_ghost_texture();

    SDL_Texture* get_player_texture();

    SDL_Texture* get_head_texture();

    SDL_Texture* get_hat_texture();

    SDL_Texture* get_game_texture();

    SDL_Texture* get_hp_texture();

    SDL_Texture* get_xp_texture();

    SDL_Texture* get_mana_texture();

    SDL_Texture* get_shop_texture();

    void load_gold();

    void load_ghost();

    void load_player(const std::string& image_path);

    void load_npcs_friendlies();

    void load_spell_effects();

    void load_items();
    
    void load_head_for_race(const std::string& race);

    void load_stats();

    void load_game_window();

    void load_shop();

    void freeSDL();

};

#endif