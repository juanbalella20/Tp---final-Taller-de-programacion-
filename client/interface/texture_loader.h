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

    std::map<std::string, SDL_Texture*> enemies_textures;
    std::map<std::string, SDL_Texture*> spell_effects_textures;

    void load_npc_enemy(const std::string& npc_name, const std::string& image_path);

public:
    TextureLoader(SDL_Window* window, SDL_Renderer* renderer);

    void load_game_logo();

    void load_npcs_enemies();

    SDL_Texture* texture_of_npc_enemy(const std::string& npc_name);

    SDL_Texture* texture_of_spell(const std::string& spell_id);

    void load_specific(const std::string& image_path, SDL_Texture* texture);

    void load_gold();

    void load_npcs_friendlies();

    void load_spell_effects();

    void freeSDL();

}

#endif