#include "texture_loader.h"

#include <SDL3_image/SDL_image.h>

TextureLoader::TextureLoader(SDL_Window* window, SDL_Renderer* renderer) :
    window(window),
    renderer(renderer) {}

void TextureLoader::load_game_logo() {
    std::string path = paths::asset("imagenes/logo.jpeg");
    SDL_Surface* icon = IMG_Load(path.c_str());
    if (icon) {
        SDL_SetWindowIcon(window, icon);
        SDL_DestroySurface(icon);
    }
}

void TextureLoader::load_npc_enemy(const std::string& npc_name, const std::string& image_path) {
    std::string path = paths::asset(image_path.c_str());
    SDL_Surface* surf = IMG_Load(path.c_str());
    if (!surf) {
        throw std::runtime_error(std::string("Loading npc surface: ") + SDL_GetError());
    }
    if (surf) {
        SDL_Texture* text = SDL_CreateTextureFromSurface(renderer, surf);
        enemies_textures[npc_name] = text;
        SDL_DestroySurface(surf);
    }
}

void TextureLoader::load_npcs_enemies() {
    load_npc("Goblin", "imagenes/goblin1.png");
    load_npc("Spider1", "imagenes/araña1.png");
    load_npc("Spider2", "imagenes/araña2.png");
    load_npc("Spider3", "imagenes/araña3.png");
    load_npc("Skeleton1", "imagenes/esqueleto1.png");
    load_npc("Skeleton2", "imagenes/esqueleto2.png");
    load_npc("Skeleton3", "imagenes/esqueleto3.png");
    load_npc("Golem1", "imagenes/golem1.png");
    load_npc("Golem2", "imagenes/golem2.png");
    load_npc("Golem3", "imagenes/golem3.png");
    load_npc("Zombie", "imagenes/zombie.png");
    load_npc("Orc", "imagenes/orco.png");
}

SDL_Texture* TextureLoader::texture_of_npc_enemy(const std::string& npc_name) {
    auto tex_it = enemies_textures.find(npc_name);
    if (tex_it == enemies_textures.end()) return nullptr;
    return tex_it->second;
}

SDL_Texture* TextureLoader::texture_of_spell(const std::string& spell_id) {
    auto tex_it = spell_effects_textures.find(spell_id);
    if (tex_it == spell_effects_textures.end()) return nullptr;
    return tex_it->second;
}

void TextureLoader::load_specific(const std::string& image_path, SDL_Texture* texture) {
    std::string path = paths::asset(image_path);
    SDL_Surface* surf = IMG_Load(path.c_str());
    if (surf) {
        texture = SDL_CreateTextureFromSurface(renderer, surf);
        SDL_DestroySurface(surf);
    }
}

void TextureLoader::load_gold() {
    load_specific("imagenes/100.png", gold_texture);
}

void TextureLoader::load_npcs_friendlies() {
    load_specific("imagenes/4055.png", seller_texture);
    load_specific("imagenes/4051.png", banker_texture);
    load_specific("imagenes/4057.png", priest_texture);
}

void TextureLoader::load_spell_effects() {
    struct Def { const char* id; const char* path; };
    const Def defs[] = {
        // Curación (flauta élfica)
        {"flauta_elfica",    "imagenes/3444.png"},
        // Flecha mágica (vara de fresno)
        {"vara_fresno",      "imagenes/2511.png"},
        // Misil (báculo nudoso)
        {"baculo_nudoso",    "imagenes/3451.png"},
        // Explosión (báculo engarzado)
        {"baculo_engarzado", "imagenes/864.png"},
    };
    for (const auto& d : defs) {
        SDL_Texture* tex = nullptr;
        load_specific(d.path, tex);
        if (!tex) continue;
        spell_effects_textures[d.id] = tex;
    }
}

void TextureLoader::freeSDL() {
    if (seller_texture) { 
        SDL_DestroyTexture(seller_texture); 
        seller_texture = nullptr; 
    }
    if (banker_texture) { 
        SDL_DestroyTexture(banker_texture); 
        banker_texture = nullptr; 
    }
    if (priest_texture) { 
        SDL_DestroyTexture(priest_texture); 
        priest_texture = nullptr;
    }

    // Texturas extra de items del piso (las que no son item_texture, p. ej. escudo).
    // Se saltea gold_texture: lo comparten las pociones del piso y se libera aparte.
    for (auto& kv : floor_item_textures) {
        if (kv.second && kv.second != item_texture && kv.second != gold_texture) {
            SDL_DestroyTexture(kv.second);
        }
    }
    floor_item_textures.clear();

    if (item_texture) {
        SDL_DestroyTexture(item_texture);
        item_texture = nullptr;
    }

    if (gold_texture) {
        SDL_DestroyTexture(gold_texture);
        gold_texture = nullptr;
    }
}