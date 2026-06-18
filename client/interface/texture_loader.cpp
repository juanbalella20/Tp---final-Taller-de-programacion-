#include "texture_loader.h"

#include <SDL3_image/SDL_image.h>
#include <stdexcept>

TextureLoader::TextureLoader(SDL_Window* window, SDL_Renderer* renderer) :
    window(window),
    renderer(renderer) {}

TextureLoader::TextureLoader(SDL_Renderer* renderer) :
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
    load_npc_enemy("Goblin", "imagenes/goblin1.png");
    load_npc_enemy("Spider1", "imagenes/araña1.png");
    load_npc_enemy("Spider2", "imagenes/araña2.png");
    load_npc_enemy("Spider3", "imagenes/araña3.png");
    load_npc_enemy("Skeleton1", "imagenes/esqueleto1.png");
    load_npc_enemy("Skeleton2", "imagenes/esqueleto2.png");
    load_npc_enemy("Skeleton3", "imagenes/esqueleto3.png");
    load_npc_enemy("Golem1", "imagenes/golem1.png");
    load_npc_enemy("Golem2", "imagenes/golem2.png");
    load_npc_enemy("Golem3", "imagenes/golem3.png");
    load_npc_enemy("Zombie", "imagenes/zombie.png");
    load_npc_enemy("Orc", "imagenes/orco.png");
}

SDL_Texture* TextureLoader::get_texture_of_npc_enemy(const std::string& npc_name) {
    auto tex_it = enemies_textures.find(npc_name);
    if (tex_it == enemies_textures.end()) return nullptr;
    return tex_it->second;
}

SDL_Texture* TextureLoader::get_texture_of_spell(const std::string& spell_id) {
    auto tex_it = spell_effects_textures.find(spell_id);
    if (tex_it == spell_effects_textures.end()) return nullptr;
    return tex_it->second;
}

SDL_Texture* TextureLoader::get_texture_of_item(const std::string& item) {
    auto tex_it = item_textures.find(item);
    if (tex_it == item_textures.end()) return nullptr;
    return tex_it->second;
}

void TextureLoader::load_specific(const std::string& image_path, SDL_Texture** texture) {
    std::string path = paths::asset(image_path);
    SDL_Surface* surf = IMG_Load(path.c_str());
    if (!surf) {
        throw std::runtime_error(std::string("Loading surface: ") + SDL_GetError());
    }
    if (surf) {
        (*texture) = SDL_CreateTextureFromSurface(renderer, surf);
        SDL_DestroySurface(surf);
    }
    if (!*texture) {
        throw std::runtime_error(std::string("Creating texture: ") + SDL_GetError());
    }
}

void TextureLoader::load_gold() {
    load_specific("imagenes/oro.png", &gold_texture);
}

void TextureLoader::load_ghost() {
    load_specific("imagenes/fantasma.png", &ghost_texture);
}

void TextureLoader::load_player(const std::string& image_path) {
    load_specific(image_path, &player_texture);
    SDL_SetTextureBlendMode(player_texture, SDL_BLENDMODE_BLEND);
}

SDL_Texture* TextureLoader::get_gold_texture() {
    return gold_texture;
}

SDL_Texture* TextureLoader::get_seller_texture() {
    return seller_texture;
}

SDL_Texture* TextureLoader::get_banker_texture() {
    return banker_texture;
}

SDL_Texture* TextureLoader::get_priest_texture() {
    return priest_texture;
}

SDL_Texture* TextureLoader::get_ghost_texture() {
    return ghost_texture;
}

SDL_Texture* TextureLoader::get_player_texture() {
    return player_texture;
}

SDL_Texture* TextureLoader::get_head_texture() {
    return head_texture;
}

SDL_Texture* TextureLoader::get_hat_texture() {
    return hat_texture;
}

void TextureLoader::load_npcs_friendlies() {
    load_specific("imagenes/4055.png", &seller_texture);
    load_specific("imagenes/4051.png", &banker_texture);
    load_specific("imagenes/4057.png", &priest_texture);
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
        load_specific(d.path, &tex);
        if (!tex) continue;
        spell_effects_textures[d.id] = tex;
    }
}

void TextureLoader::load_items() {
    struct ItemIcon { const char* id; const char* path; };
    const ItemIcon icons[] = {
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
        // Escudo de tortuga (+ alias histórico "escudo": misma imagen).
        {"escudo_tortuga",   "imagenes/escudo-tortuga.png"},
        {"escudo",           "imagenes/escudo-tortuga.png"},
        {"espada",           "imagenes/espada.png"},
        {"escudo_hierro",    "imagenes/escudo-hierro.png"},
        {"pocion_vida",      "imagenes/pocion-vida.png"},
        {"pocion_mana",      "imagenes/pocion-mana.png"}
    };
    for (const auto& ic : icons) {
        SDL_Texture* tex = nullptr;
        load_specific(ic.path, &tex);
        if (!tex) continue;
        SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_BLEND);
        item_textures[ic.id] = tex;
    }
}

void TextureLoader::load_head_for_race(const std::string& race) {
    SDL_Surface* head_surf = nullptr;
    SDL_Surface* hat_surf = nullptr;

    std::string path;
    if (race == "human") {
        path = paths::asset("imagenes/420.png");
        head_surf = IMG_Load(path.c_str());
    } else if (race == "elf") {
        path = paths::asset("imagenes/422.png");
        head_surf = IMG_Load(path.c_str());
    } else if (race == "dwarf") {
        path = paths::asset("imagenes/426.png");
        head_surf = IMG_Load(path.c_str());
    } else if (race == "gnome") {
        path = paths::asset("imagenes/426.png");
        head_surf = IMG_Load(path.c_str());
    
        load_specific("imagenes/437.png", &hat_texture);
    }

    if (!head_surf) {
        throw std::runtime_error(std::string("Loading head surface: ") + SDL_GetError());
    }
    head_texture = SDL_CreateTextureFromSurface(renderer, head_surf);
    SDL_DestroySurface(head_surf);
    if (!head_texture) {
        throw std::runtime_error(std::string("Creating head texture: ") + SDL_GetError());
    }
}

void TextureLoader::free_texture(SDL_Texture** texture) {
    if (*texture) { 
        SDL_DestroyTexture(*texture); 
        *texture = nullptr;
    }
}

void TextureLoader::freeSDL() {
    free_texture(&seller_texture);
    free_texture(&banker_texture);
    free_texture(&priest_texture);
    free_texture(&player_texture);
    free_texture(&ghost_texture);
    free_texture(&gold_texture);
    free_texture(&hat_texture);
    free_texture(&head_texture);

    // Texturas extra de items del piso (las que no son item_texture, p. ej. escudo).
    // Se saltea gold_texture: lo comparten las pociones del piso y se libera aparte.
    for (auto& kv : item_textures) {
        if (kv.second) {
            SDL_DestroyTexture(kv.second);
        }
    }
    item_textures.clear();

    for (auto& kv : spell_effects_textures) {
        if (kv.second) {
            SDL_DestroyTexture(kv.second);
        }
    }
    spell_effects_textures.clear();

    for (auto& kv : enemies_textures) {
        if (kv.second) {
            SDL_DestroyTexture(kv.second);
        }
    }
    enemies_textures.clear();
}