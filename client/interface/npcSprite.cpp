#include "npcSprite.h"

#include <SDL3/SDL.h>
#include <vector>
#include <map>
#include <string>

NpcSprite::NpcSprite(SDL_Renderer* renderer, SDL_Texture* texture,
              int tile_x, int tile_y, int tile_size)
        : renderer(renderer), texture(texture),
          tile_x(tile_x), tile_y(tile_y), tile_size(tile_size) {

    npcs_back_povs();
    npcs_front_povs();
    npcs_right_povs();
    npcs_left_povs();
}

void NpcSprite::draw(const Camera& camera, SDL_FRect src_crop) {
    const float wx = static_cast<float>(tile_x * tile_size);
    const float wy = static_cast<float>(tile_y * tile_size);
    SDL_FRect dst{
        camera.world_to_screen_x(wx),
        camera.world_to_screen_y(wy),
        static_cast<float>(tile_size),
        static_cast<float>(tile_size)
    };
    // Si el crop tiene area, recorta ese tile del spritesheet 
    // si viene vacio, dibuja la textura completa.
    // TODO: hacer movimientos como en displayPlayer.cpp
    const bool has_crop = src_crop.w > 0.0f && src_crop.h > 0.0f;
    SDL_RenderTexture(renderer, texture, has_crop ? &src_crop : nullptr, &dst);
}

SDL_FRect NpcSprite::back_pov(const std::string& npc_name) {
    return npc_back_pov(npc_name);
}

SDL_FRect NpcSprite::front_pov(const std::string& npc_name) {
    return npc_front_pov(npc_name);
}

SDL_FRect NpcSprite::right_pov(const std::string& npc_name) {
    return npc_right_pov(npc_name);
}

SDL_FRect NpcSprite::left_pov(const std::string& npc_name) {
    return npc_left_pov(npc_name);
}

SDL_FRect NpcSprite::current_frame(const std::vector<SDL_FRect>& frames) {
    int current_frame = walk_frame % 4;

    SDL_FRect frame = frames[current_frame];
    walk_frame = (walk_frame + 1) % 4;
    return frame;
}

SDL_FRect NpcSprite::npc_back_pov(const std::string& name) {
    const std::vector<SDL_FRect>& frames = back_povs.at(name);

    return current_frame(frames);
}

SDL_FRect NpcSprite::npc_front_pov(const std::string& name) {
    const std::vector<SDL_FRect>& frames = front_povs.at(name);

    return current_frame(frames);
}

SDL_FRect NpcSprite::npc_right_pov(const std::string& name) {
    const std::vector<SDL_FRect>& frames = right_povs.at(name);

    return current_frame(frames);
}

SDL_FRect NpcSprite::npc_left_pov(const std::string& name) {
    const std::vector<SDL_FRect>& frames = left_povs.at(name);

    return current_frame(frames);
}

void NpcSprite::back_pov_goblin() {
    std::vector<SDL_FRect> frames;
    frames.push_back({0.0f, 0.0f, 32.0f, 32.0f});
    frames.push_back({0.0f, 0.0f, 32.0f, 32.0f});
    frames.push_back({0.0f, 0.0f, 32.0f, 32.0f});
    frames.push_back({0.0f, 0.0f, 32.0f, 32.0f});

    back_povs["Goblin"] = frames;
}

void NpcSprite::back_pov_zombie() {
    std::vector<SDL_FRect> frames;
    frames.push_back({0.0f, 0.0f, 32.0f, 32.0f});
    frames.push_back({0.0f, 0.0f, 32.0f, 32.0f});
    frames.push_back({0.0f, 0.0f, 32.0f, 32.0f});
    frames.push_back({0.0f, 0.0f, 32.0f, 32.0f});

    back_povs["Zombie"] = frames;
}

void NpcSprite::back_pov_spiders() {
    std::vector<SDL_FRect> frames1;
    std::vector<SDL_FRect> frames2;
    std::vector<SDL_FRect> frames3;

    frames1.push_back({0.0f, 0.0f, 64.0f, 64.0f});
    frames1.push_back({0.0f, 0.0f, 32.0f, 32.0f});
    frames1.push_back({0.0f, 0.0f, 32.0f, 32.0f});
    frames1.push_back({0.0f, 0.0f, 32.0f, 32.0f});

    frames2.push_back({0.0f, 14.0f, 95.0f, 65.0f});
    frames2.push_back({0.0f, 0.0f, 32.0f, 32.0f});
    frames2.push_back({0.0f, 0.0f, 32.0f, 32.0f});
    frames2.push_back({0.0f, 0.0f, 32.0f, 32.0f});

    frames3.push_back({20.0f, 69.0f, 84.0f, 57.0f});
    frames3.push_back({0.0f, 0.0f, 32.0f, 32.0f});
    frames3.push_back({0.0f, 0.0f, 32.0f, 32.0f});
    frames3.push_back({0.0f, 0.0f, 32.0f, 32.0f});

    back_povs["Spider1"] = frames1;
    back_povs["Spider2"] = frames2;
    back_povs["Spider3"] = frames3;
}

void NpcSprite::back_pov_skeletons() {
    std::vector<SDL_FRect> frames1;
    std::vector<SDL_FRect> frames2;
    std::vector<SDL_FRect> frames3;
    
    frames1.push_back({0.0f, 0.0f, 32.0f, 32.0f});
    frames1.push_back({0.0f, 0.0f, 32.0f, 32.0f});
    frames1.push_back({0.0f, 0.0f, 32.0f, 32.0f});
    frames1.push_back({0.0f, 0.0f, 32.0f, 32.0f});

    frames2.push_back({0.0f, 0.0f, 32.0f, 32.0f});
    frames2.push_back({0.0f, 0.0f, 32.0f, 32.0f});
    frames2.push_back({0.0f, 0.0f, 32.0f, 32.0f});
    frames2.push_back({0.0f, 0.0f, 32.0f, 32.0f});

    frames3.push_back({0.0f, 0.0f, 32.0f, 32.0f});
    frames3.push_back({0.0f, 0.0f, 32.0f, 32.0f});
    frames3.push_back({0.0f, 0.0f, 32.0f, 32.0f});
    frames3.push_back({0.0f, 0.0f, 32.0f, 32.0f});

    back_povs["Skeleton1"] = frames1;
    back_povs["Skeleton2"] = frames2;
    back_povs["Skeleton3"] = frames3;
}

void NpcSprite::back_pov_golems() {
    std::vector<SDL_FRect> frames1;
    std::vector<SDL_FRect> frames2;
    std::vector<SDL_FRect> frames3;
    
    frames1.push_back({0.0f, 0.0f, 32.0f, 32.0f});
    frames1.push_back({0.0f, 0.0f, 32.0f, 32.0f});
    frames1.push_back({0.0f, 0.0f, 32.0f, 32.0f});
    frames1.push_back({0.0f, 0.0f, 32.0f, 32.0f});

    frames2.push_back({0.0f, 0.0f, 32.0f, 32.0f});
    frames2.push_back({0.0f, 0.0f, 32.0f, 32.0f});
    frames2.push_back({0.0f, 0.0f, 32.0f, 32.0f});
    frames2.push_back({0.0f, 0.0f, 32.0f, 32.0f});

    frames3.push_back({0.0f, 0.0f, 32.0f, 32.0f});
    frames3.push_back({0.0f, 0.0f, 32.0f, 32.0f});
    frames3.push_back({0.0f, 0.0f, 32.0f, 32.0f});
    frames3.push_back({0.0f, 0.0f, 32.0f, 32.0f});

    back_povs["Golem1"] = frames1;
    back_povs["Golem2"] = frames2;
    back_povs["Golem3"] = frames3;
}

void NpcSprite::front_pov_goblin() {
    std::vector<SDL_FRect> frames;
    frames.push_back({0.0f, 0.0f, 32.0f, 32.0f});
    frames.push_back({0.0f, 0.0f, 32.0f, 32.0f});
    frames.push_back({0.0f, 0.0f, 32.0f, 32.0f});
    frames.push_back({0.0f, 0.0f, 32.0f, 32.0f});

    front_povs["Goblin"] = frames;
}

void NpcSprite::front_pov_zombie() {
    std::vector<SDL_FRect> frames;
    frames.push_back({0.0f, 0.0f, 32.0f, 32.0f});
    frames.push_back({0.0f, 0.0f, 32.0f, 32.0f});
    frames.push_back({0.0f, 0.0f, 32.0f, 32.0f});
    frames.push_back({0.0f, 0.0f, 32.0f, 32.0f});

    front_povs["Zombie"] = frames;
}

void NpcSprite::front_pov_spiders() {
    std::vector<SDL_FRect> frames1;
    std::vector<SDL_FRect> frames2;
    std::vector<SDL_FRect> frames3;

    frames1.push_back({0.0f, 0.0f, 64.0f, 64.0f});
    frames1.push_back({0.0f, 0.0f, 32.0f, 32.0f});
    frames1.push_back({0.0f, 0.0f, 32.0f, 32.0f});
    frames1.push_back({0.0f, 0.0f, 32.0f, 32.0f});

    frames2.push_back({0.0f, 14.0f, 95.0f, 65.0f});
    frames2.push_back({0.0f, 0.0f, 32.0f, 32.0f});
    frames2.push_back({0.0f, 0.0f, 32.0f, 32.0f});
    frames2.push_back({0.0f, 0.0f, 32.0f, 32.0f});

    frames3.push_back({20.0f, 69.0f, 84.0f, 57.0f});
    frames3.push_back({0.0f, 0.0f, 32.0f, 32.0f});
    frames3.push_back({0.0f, 0.0f, 32.0f, 32.0f});
    frames3.push_back({0.0f, 0.0f, 32.0f, 32.0f});

    front_povs["Spider1"] = frames1;
    front_povs["Spider2"] = frames2;
    front_povs["Spider3"] = frames3;
}

void NpcSprite::front_pov_skeletons() {
    std::vector<SDL_FRect> frames1;
    std::vector<SDL_FRect> frames2;
    std::vector<SDL_FRect> frames3;
    
    frames1.push_back({0.0f, 0.0f, 32.0f, 32.0f});
    frames1.push_back({0.0f, 0.0f, 32.0f, 32.0f});
    frames1.push_back({0.0f, 0.0f, 32.0f, 32.0f});
    frames1.push_back({0.0f, 0.0f, 32.0f, 32.0f});

    frames2.push_back({0.0f, 0.0f, 32.0f, 32.0f});
    frames2.push_back({0.0f, 0.0f, 32.0f, 32.0f});
    frames2.push_back({0.0f, 0.0f, 32.0f, 32.0f});
    frames2.push_back({0.0f, 0.0f, 32.0f, 32.0f});

    frames3.push_back({0.0f, 0.0f, 32.0f, 32.0f});
    frames3.push_back({0.0f, 0.0f, 32.0f, 32.0f});
    frames3.push_back({0.0f, 0.0f, 32.0f, 32.0f});
    frames3.push_back({0.0f, 0.0f, 32.0f, 32.0f});

    front_povs["Skeleton1"] = frames1;
    front_povs["Skeleton2"] = frames2;
    front_povs["Skeleton3"] = frames3;
}

void NpcSprite::front_pov_golems() {
    std::vector<SDL_FRect> frames1;
    std::vector<SDL_FRect> frames2;
    std::vector<SDL_FRect> frames3;
    
    frames1.push_back({0.0f, 0.0f, 32.0f, 32.0f});
    frames1.push_back({0.0f, 0.0f, 32.0f, 32.0f});
    frames1.push_back({0.0f, 0.0f, 32.0f, 32.0f});
    frames1.push_back({0.0f, 0.0f, 32.0f, 32.0f});

    frames2.push_back({0.0f, 0.0f, 32.0f, 32.0f});
    frames2.push_back({0.0f, 0.0f, 32.0f, 32.0f});
    frames2.push_back({0.0f, 0.0f, 32.0f, 32.0f});
    frames2.push_back({0.0f, 0.0f, 32.0f, 32.0f});

    frames3.push_back({0.0f, 0.0f, 32.0f, 32.0f});
    frames3.push_back({0.0f, 0.0f, 32.0f, 32.0f});
    frames3.push_back({0.0f, 0.0f, 32.0f, 32.0f});
    frames3.push_back({0.0f, 0.0f, 32.0f, 32.0f});

    front_povs["Golem1"] = frames1;
    front_povs["Golem2"] = frames2;
    front_povs["Golem3"] = frames3;
}

void NpcSprite::right_pov_goblin() {
    std::vector<SDL_FRect> frames;
    frames.push_back({0.0f, 0.0f, 32.0f, 32.0f});
    frames.push_back({0.0f, 0.0f, 32.0f, 32.0f});
    frames.push_back({0.0f, 0.0f, 32.0f, 32.0f});
    frames.push_back({0.0f, 0.0f, 32.0f, 32.0f});

    right_povs["Goblin"] = frames;
}

void NpcSprite::right_pov_zombie() {
    std::vector<SDL_FRect> frames;
    frames.push_back({0.0f, 0.0f, 32.0f, 32.0f});
    frames.push_back({0.0f, 0.0f, 32.0f, 32.0f});
    frames.push_back({0.0f, 0.0f, 32.0f, 32.0f});
    frames.push_back({0.0f, 0.0f, 32.0f, 32.0f});

    right_povs["Zombie"] = frames;
}

void NpcSprite::right_pov_spiders() {
    std::vector<SDL_FRect> frames1;
    std::vector<SDL_FRect> frames2;
    std::vector<SDL_FRect> frames3;

    frames1.push_back({0.0f, 0.0f, 64.0f, 64.0f});
    frames1.push_back({0.0f, 0.0f, 32.0f, 32.0f});
    frames1.push_back({0.0f, 0.0f, 32.0f, 32.0f});
    frames1.push_back({0.0f, 0.0f, 32.0f, 32.0f});

    frames2.push_back({0.0f, 14.0f, 95.0f, 65.0f});
    frames2.push_back({0.0f, 0.0f, 32.0f, 32.0f});
    frames2.push_back({0.0f, 0.0f, 32.0f, 32.0f});
    frames2.push_back({0.0f, 0.0f, 32.0f, 32.0f});

    frames3.push_back({20.0f, 69.0f, 84.0f, 57.0f});
    frames3.push_back({0.0f, 0.0f, 32.0f, 32.0f});
    frames3.push_back({0.0f, 0.0f, 32.0f, 32.0f});
    frames3.push_back({0.0f, 0.0f, 32.0f, 32.0f});

    right_povs["Spider1"] = frames1;
    right_povs["Spider2"] = frames2;
    right_povs["Spider3"] = frames3;
}

void NpcSprite::right_pov_skeletons() {
    std::vector<SDL_FRect> frames1;
    std::vector<SDL_FRect> frames2;
    std::vector<SDL_FRect> frames3;
    
    frames1.push_back({0.0f, 0.0f, 32.0f, 32.0f});
    frames1.push_back({0.0f, 0.0f, 32.0f, 32.0f});
    frames1.push_back({0.0f, 0.0f, 32.0f, 32.0f});
    frames1.push_back({0.0f, 0.0f, 32.0f, 32.0f});

    frames2.push_back({0.0f, 0.0f, 32.0f, 32.0f});
    frames2.push_back({0.0f, 0.0f, 32.0f, 32.0f});
    frames2.push_back({0.0f, 0.0f, 32.0f, 32.0f});
    frames2.push_back({0.0f, 0.0f, 32.0f, 32.0f});

    frames3.push_back({0.0f, 0.0f, 32.0f, 32.0f});
    frames3.push_back({0.0f, 0.0f, 32.0f, 32.0f});
    frames3.push_back({0.0f, 0.0f, 32.0f, 32.0f});
    frames3.push_back({0.0f, 0.0f, 32.0f, 32.0f});

    right_povs["Skeleton1"] = frames1;
    right_povs["Skeleton2"] = frames2;
    right_povs["Skeleton3"] = frames3;
}

void NpcSprite::right_pov_golems() {
    std::vector<SDL_FRect> frames1;
    std::vector<SDL_FRect> frames2;
    std::vector<SDL_FRect> frames3;
    
    frames1.push_back({0.0f, 0.0f, 32.0f, 32.0f});
    frames1.push_back({0.0f, 0.0f, 32.0f, 32.0f});
    frames1.push_back({0.0f, 0.0f, 32.0f, 32.0f});
    frames1.push_back({0.0f, 0.0f, 32.0f, 32.0f});

    frames2.push_back({0.0f, 0.0f, 32.0f, 32.0f});
    frames2.push_back({0.0f, 0.0f, 32.0f, 32.0f});
    frames2.push_back({0.0f, 0.0f, 32.0f, 32.0f});
    frames2.push_back({0.0f, 0.0f, 32.0f, 32.0f});

    frames3.push_back({0.0f, 0.0f, 32.0f, 32.0f});
    frames3.push_back({0.0f, 0.0f, 32.0f, 32.0f});
    frames3.push_back({0.0f, 0.0f, 32.0f, 32.0f});
    frames3.push_back({0.0f, 0.0f, 32.0f, 32.0f});

    right_povs["Golem1"] = frames1;
    right_povs["Golem2"] = frames2;
    right_povs["Golem3"] = frames3;
}

void NpcSprite::npcs_back_povs() {
    back_pov_goblin();
    back_pov_zombie();
    back_pov_spiders();
    back_pov_skeletons();
    back_pov_golems();
}

void NpcSprite::npcs_front_povs() {
    front_pov_goblin();
    front_pov_zombie();
    front_pov_spiders();
    front_pov_skeletons();
    front_pov_golems();
}

void NpcSprite::npcs_right_povs() {
    right_pov_goblin();
    right_pov_zombie();
    right_pov_spiders();
    right_pov_skeletons();
    right_pov_golems();
}

void NpcSprite::npcs_left_povs() {
    left_pov_goblin();
    left_pov_zombie();
    left_pov_spiders();
    left_pov_skeletons();
    left_pov_golems();
}
