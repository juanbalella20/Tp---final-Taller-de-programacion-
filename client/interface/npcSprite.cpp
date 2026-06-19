#include "npcSprite.h"

#include <SDL3/SDL.h>
#include <vector>
#include <map>
#include <string>

NpcSprite::NpcSprite(SDL_Renderer* renderer, SDL_Texture* texture,
              int tile_x, int tile_y, int tile_size, const std::string& npc_name,
              int frame_index)
        : renderer(renderer), texture(texture),
          tile_x(tile_x), tile_y(tile_y), tile_size(tile_size), frame_index(frame_index),
          size_scale(size_scale_for(npc_name)) {

    npcs_back_povs();
    npcs_front_povs();
    npcs_right_povs();
    npcs_left_povs();
}

void NpcSprite::draw(const Camera& camera, SDL_FRect src_crop) {
    const float wx = static_cast<float>(tile_x * tile_size);
    const float wy = static_cast<float>(tile_y * tile_size);
    const float rendered_size = static_cast<float>(tile_size) * size_scale;
    const float foot_offset = static_cast<float>(tile_size) - rendered_size;
    SDL_FRect dst{
        camera.world_to_screen_x(wx),
        camera.world_to_screen_y(wy) + foot_offset,
        rendered_size,
        rendered_size
    };
    // Si el crop tiene area, recorta ese tile del spritesheet 
    // si viene vacio, dibuja la textura completa.
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

float NpcSprite::size_scale_for(const std::string& name) {
    static const std::map<std::string, float> scales = {
        {"Goblin", 0.7f},
        {"Spider1", 0.7f}
    };
    auto it = scales.find(name);
    return (it != scales.end()) ? it->second : 1.0f;
}

SDL_FRect NpcSprite::current_frame(const std::vector<SDL_FRect>& frames) {
    if (frames.empty()) return {};

    return frames[static_cast<size_t>(frame_index) % frames.size()];
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
    frames.push_back({1.0f, 35.0f, 21.0f, 26.0f});
    frames.push_back({24.0f, 35.0f, 23.0f, 26.0f});
    frames.push_back({125.0f, 35.0f, 22.0f, 26.0f});

    back_povs["Goblin"] = frames;
}

void NpcSprite::back_pov_zombie() {
    std::vector<SDL_FRect> frames;
    frames.push_back({1.0f, 48.0f, 24.0f, 44.0f});
    frames.push_back({25.0f, 48.0f, 20.0f, 44.0f});
    frames.push_back({119.0f, 48.0f, 20.0f, 44.0f});

    back_povs["Zombie"] = frames;
}

void NpcSprite::back_pov_orc() {
    std::vector<SDL_FRect> frames;
    frames.push_back({1.0f, 112.0f, 52.0f, 75.0f});
    frames.push_back({62.0f, 112.0f, 49.0f, 75.0f});
    frames.push_back({231.0f, 112.0f, 49.0f, 75.0f});

    back_povs["Orc"] = frames;
}

void NpcSprite::back_pov_spiders() {
    std::vector<SDL_FRect> frames1;
    std::vector<SDL_FRect> frames2;
    std::vector<SDL_FRect> frames3;

    frames1.push_back({3.0f, 92.0f, 60.0f, 39.0f});
    frames1.push_back({63.0f, 92.0f, 62.0f, 39.0f});
    frames1.push_back({386.0f, 92.0f, 57.0f, 39.0f});

    frames2.push_back({2.0f, 505.0f, 90.0f, 64.0f});
    frames2.push_back({95.0f, 505.0f, 95.0f, 64.0f});
    frames2.push_back({286.0f, 505.0f, 97.0f, 64.0f});

    frames3.push_back({13.0f, 705.0f, 95.0f, 63.0f});
    frames3.push_back({142.0f, 705.0f, 94.0f, 63.0f});
    frames3.push_back({267.0f, 705.0f, 98.0f, 63.0f});

    back_povs["Spider1"] = frames1;
    back_povs["Spider2"] = frames2;
    back_povs["Spider3"] = frames3;
}

void NpcSprite::back_pov_skeletons() {
    std::vector<SDL_FRect> frames1;
    std::vector<SDL_FRect> frames2;
    std::vector<SDL_FRect> frames3;
    
    frames1.push_back({107.0f, 489.0f, 73.0f, 78.0f});
    frames1.push_back({207.0f, 489.0f, 65.0f, 78.0f});
    frames1.push_back({297.0f, 489.0f, 78.0f, 78.0f});

    frames2.push_back({7.0f, 67.0f, 19.0f, 34.0f});
    frames2.push_back({37.0f, 67.0f, 18.0f, 34.0f});
    frames2.push_back({167.0f, 67.0f, 18.0f, 34.0f});

    frames3.push_back({0.0f, 54.0f, 25.0f, 49.0f});
    frames3.push_back({25.0f, 54.0f, 25.0f, 49.0f});
    frames3.push_back({50.0f, 54.0f, 25.0f, 49.0f});
    frames3.push_back({75.0f, 54.0f, 25.0f, 49.0f});
    frames3.push_back({100.0f, 54.0f, 25.0f, 49.0f});
    frames3.push_back({125.0f, 54.0f, 25.0f, 49.0f});

    back_povs["Skeleton1"] = frames1;
    back_povs["Skeleton2"] = frames2;
    back_povs["Skeleton3"] = frames3;
}

void NpcSprite::back_pov_golems() {
    std::vector<SDL_FRect> frames1;
    std::vector<SDL_FRect> frames2;
    
    frames1.push_back({21.0f, 161.0f, 121.0f, 153.0f});
    frames1.push_back({163.0f, 161.0f, 151.0f, 153.0f});
    frames1.push_back({641.0f, 161.0f, 155.0f, 153.0f});

    frames2.push_back({6.0f, 227.0f, 142.0f, 152.0f});
    frames2.push_back({157.0f, 227.0f, 170.0f, 152.0f});
    frames2.push_back({636.0f, 227.0f, 167.0f, 152.0f});
    
    back_povs["Golem1"] = frames1;
    back_povs["Golem2"] = frames1;
    back_povs["Golem3"] = frames2;
}

void NpcSprite::front_pov_goblin() {
    std::vector<SDL_FRect> frames;
    frames.push_back({1.0f, 4.0f, 21.0f, 27.0f});
    frames.push_back({24.0f, 4.0f, 23.0f, 27.0f});
    frames.push_back({125.0f, 4.0f, 22.0f, 27.0f});

    front_povs["Goblin"] = frames;
}

void NpcSprite::front_pov_zombie() {
    std::vector<SDL_FRect> frames;
    frames.push_back({1.0f, 1.0f, 24.0f, 44.0f});
    frames.push_back({25.0f, 1.0f, 20.0f, 44.0f});
    frames.push_back({119.0f, 1.0f, 20.0f, 44.0f});

    front_povs["Zombie"] = frames;
}

void NpcSprite::front_pov_orc() {
    std::vector<SDL_FRect> frames;
    frames.push_back({1.0f, 15.0f, 52.0f, 75.0f});
    frames.push_back({62.0f, 15.0f, 49.0f, 75.0f});
    frames.push_back({231.0f, 15.0f, 49.0f, 75.0f});

    front_povs["Orc"] = frames;
}

void NpcSprite::front_pov_spiders() {
    std::vector<SDL_FRect> frames1;
    std::vector<SDL_FRect> frames2;
    std::vector<SDL_FRect> frames3;

    frames1.push_back({3.0f, 28.0f, 60.0f, 37.0f});
    frames1.push_back({63.0f, 28.0f, 62.0f, 37.0f});
    frames1.push_back({386.0f, 28.0f, 57.0f, 37.0f});

    frames2.push_back({2.0f, 396.0f, 90.0f, 76.0f});
    frames2.push_back({95.0f, 396.0f, 95.0f, 76.0f});
    frames2.push_back({286.0f, 396.0f, 97.0f, 76.0f});

    frames3.push_back({13.0f, 579.0f, 95.0f, 62.0f});
    frames3.push_back({142.0f, 579.0f, 94.0f, 62.0f});
    frames3.push_back({267.0f, 579.0f, 98.0f, 62.0f});

    front_povs["Spider1"] = frames1;
    front_povs["Spider2"] = frames2;
    front_povs["Spider3"] = frames3;
}

void NpcSprite::front_pov_skeletons() {
    std::vector<SDL_FRect> frames1;
    std::vector<SDL_FRect> frames2;
    std::vector<SDL_FRect> frames3;
    
    frames1.push_back({107.0f, 392.0f, 73.0f, 84.0f});
    frames1.push_back({207.0f, 392.0f, 65.0f, 84.0f});
    frames1.push_back({297.0f, 392.0f, 78.0f, 84.0f});

    frames2.push_back({7.0f, 16.0f, 19.0f, 34.0f});
    frames2.push_back({37.0f, 16.0f, 18.0f, 34.0f});
    frames2.push_back({167.0f, 16.0f, 18.0f, 34.0f});

    frames3.push_back({0.0f, 1.0f, 25.0f, 51.0f});
    frames3.push_back({25.0f, 1.0f, 25.0f, 51.0f});
    frames3.push_back({50.0f, 1.0f, 25.0f, 51.0f});
    frames3.push_back({75.0f, 1.0f, 25.0f, 51.0f});
    frames3.push_back({100.0f, 1.0f, 25.0f, 51.0f});
    frames3.push_back({125.0f, 1.0f, 25.0f, 51.0f});
    
    front_povs["Skeleton1"] = frames1;
    front_povs["Skeleton2"] = frames2;
    front_povs["Skeleton3"] = frames3;
}

void NpcSprite::front_pov_golems() {
    std::vector<SDL_FRect> frames1;
    std::vector<SDL_FRect> frames2;
    
    frames1.push_back({21.0f, 10.0f, 121.0f, 148.0f});
    frames1.push_back({163.0f, 10.0f, 151.0f, 148.0f});
    frames1.push_back({641.0f, 10.0f, 155.0f, 148.0f});

    frames2.push_back({6.0f, 16.0f, 142.0f, 161.0f});
    frames2.push_back({157.0f, 16.0f, 170.0f, 161.0f});
    frames2.push_back({636.0f, 16.0f, 167.0f, 161.0f});
    
    front_povs["Golem1"] = frames1;
    front_povs["Golem2"] = frames1;
    front_povs["Golem3"] = frames2;
}

void NpcSprite::right_pov_goblin() {
    std::vector<SDL_FRect> frames;
    frames.push_back({1.0f, 100.0f, 21.0f, 23.0f});
    frames.push_back({24.0f, 100.0f, 23.0f, 23.0f});
    frames.push_back({125.0f, 100.0f, 22.0f, 23.0f});

    right_povs["Goblin"] = frames;
}

void NpcSprite::right_pov_zombie() {
    std::vector<SDL_FRect> frames;
    frames.push_back({1.0f, 143.0f, 24.0f, 46.0f});
    frames.push_back({25.0f, 143.0f, 20.0f, 46.0f});
    frames.push_back({47.0f, 143.0f, 25.0f, 46.0f});

    right_povs["Zombie"] = frames;
}

void NpcSprite::right_pov_orc() {
    std::vector<SDL_FRect> frames;
    frames.push_back({1.0f, 313.0f, 52.0f, 74.0f});
    frames.push_back({62.0f, 313.0f, 49.0f, 74.0f});
    frames.push_back({231.0f, 313.0f, 49.0f, 74.0f});

    right_povs["Orc"] = frames;
}

void NpcSprite::right_pov_spiders() {
    std::vector<SDL_FRect> frames1;
    std::vector<SDL_FRect> frames2;
    std::vector<SDL_FRect> frames3;

    frames1.push_back({3.0f, 217.0f, 60.0f, 39.0f});
    frames1.push_back({63.0f, 217.0f, 62.0f, 39.0f});
    frames1.push_back({386.0f, 217.0f, 57.0f, 39.0f});

    frames2.push_back({2.0f, 681.0f, 94.0f, 72.0f});
    frames2.push_back({95.0f, 681.0f, 95.0f, 72.0f});
    frames2.push_back({286.0f, 681.0f, 97.0f, 72.0f});

    frames3.push_back({13.0f, 968.0f, 95.0f, 54.0f});
    frames3.push_back({142.0f, 968.0f, 94.0f, 54.0f});
    frames3.push_back({267.0f, 968.0f, 98.0f, 54.0f});

    right_povs["Spider1"] = frames1;
    right_povs["Spider2"] = frames2;
    right_povs["Spider3"] = frames3;
}

void NpcSprite::right_pov_skeletons() {
    std::vector<SDL_FRect> frames1;
    std::vector<SDL_FRect> frames2;
    std::vector<SDL_FRect> frames3;
    
    frames1.push_back({107.0f, 685.0f, 73.0f, 80.0f});
    frames1.push_back({207.0f, 685.0f, 65.0f, 80.0f});
    frames1.push_back({297.0f, 685.0f, 78.0f, 80.0f});

    frames2.push_back({7.0f, 167.0f, 19.0f, 34.0f});
    frames2.push_back({37.0f, 167.0f, 18.0f, 34.0f});
    frames2.push_back({167.0f, 167.0f, 18.0f, 34.0f});

    frames3.push_back({0.0f, 157.0f, 25.0f, 51.0f});
    frames3.push_back({25.0f, 157.0f, 25.0f, 51.0f});
    frames3.push_back({50.0f, 157.0f, 25.0f, 51.0f});
    frames3.push_back({75.0f, 157.0f, 25.0f, 51.0f});
    frames3.push_back({100.0f, 157.0f, 25.0f, 51.0f});
    
    right_povs["Skeleton1"] = frames1;
    right_povs["Skeleton2"] = frames2;
    right_povs["Skeleton3"] = frames3;
}

void NpcSprite::right_pov_golems() {
    std::vector<SDL_FRect> frames1;
    std::vector<SDL_FRect> frames2;
    
    frames1.push_back({21.0f, 487.0f, 121.0f, 156.0f});
    frames1.push_back({163.0f, 487.0f, 151.0f, 156.0f});
    frames1.push_back({641.0f, 487.0f, 155.0f, 156.0f});
    
    frames2.push_back({6.0f, 612.0f, 142.0f, 160.0f});
    frames2.push_back({157.0f, 612.0f, 170.0f, 160.0f});
    frames2.push_back({636.0f, 612.0f, 167.0f, 160.0f});
    
    right_povs["Golem1"] = frames1;
    right_povs["Golem2"] = frames1;
    right_povs["Golem3"] = frames2;
}

void NpcSprite::left_pov_goblin() {
    std::vector<SDL_FRect> frames;
    frames.push_back({1.0f, 68.0f, 21.0f, 24.0f});
    frames.push_back({24.0f, 68.0f, 23.0f, 24.0f});
    frames.push_back({125.0f, 68.0f, 22.0f, 24.0f});

    left_povs["Goblin"] = frames;
}

void NpcSprite::left_pov_zombie() {
    std::vector<SDL_FRect> frames;
    frames.push_back({1.0f, 96.0f, 24.0f, 45.0f});
    frames.push_back({25.0f, 96.0f, 20.0f, 45.0f});
    frames.push_back({45.0f, 95.0f, 23.0f, 46.0f});

    left_povs["Zombie"] = frames;
}

void NpcSprite::left_pov_orc() {
    std::vector<SDL_FRect> frames;
    frames.push_back({1.0f, 214.0f, 52.0f, 73.0f});
    frames.push_back({62.0f, 214.0f, 49.0f, 73.0f});
    frames.push_back({231.0f, 214.0f, 49.0f, 73.0f});

    left_povs["Orc"] = frames;
}

void NpcSprite::left_pov_spiders() {
    std::vector<SDL_FRect> frames1;
    std::vector<SDL_FRect> frames2;
    std::vector<SDL_FRect> frames3;

    frames1.push_back({3.0f, 152.0f, 60.0f, 40.0f});
    frames1.push_back({63.0f, 152.0f, 62.0f, 40.0f});
    frames1.push_back({386.0f, 152.0f, 57.0f, 40.0f});

    frames2.push_back({0.0f, 582.0f, 92.0f, 74.0f});
    frames2.push_back({95.0f, 582.0f, 95.0f, 74.0f});
    frames2.push_back({286.0f, 582.0f, 97.0f, 74.0f});

    frames3.push_back({13.0f, 839.0f, 95.0f, 58.0f});
    frames3.push_back({142.0f, 839.0f, 94.0f, 58.0f});
    frames3.push_back({267.0f, 839.0f, 98.0f, 58.0f});

    left_povs["Spider1"] = frames1;
    left_povs["Spider2"] = frames2;
    left_povs["Spider3"] = frames3;
}

void NpcSprite::left_pov_skeletons() {
    std::vector<SDL_FRect> frames1;
    std::vector<SDL_FRect> frames2;
    std::vector<SDL_FRect> frames3;
    
    frames1.push_back({107.0f, 590.0f, 73.0f, 83.0f});
    frames1.push_back({207.0f, 590.0f, 65.0f, 83.0f});
    frames1.push_back({297.0f, 590.0f, 78.0f, 83.0f});

    frames2.push_back({7.0f, 118.0f, 19.0f, 34.0f});
    frames2.push_back({37.0f, 118.0f, 18.0f, 34.0f});
    frames2.push_back({167.0f, 118.0f, 18.0f, 34.0f});

    frames3.push_back({0.0f, 105.0f, 25.0f, 51.0f});
    frames3.push_back({25.0f, 105.0f, 25.0f, 51.0f});
    frames3.push_back({50.0f, 105.0f, 25.0f, 51.0f});
    frames3.push_back({75.0f, 105.0f, 25.0f, 51.0f});
    frames3.push_back({100.0f, 105.0f, 25.0f, 51.0f});
    
    left_povs["Skeleton1"] = frames1;
    left_povs["Skeleton2"] = frames2;
    left_povs["Skeleton3"] = frames3;
}

void NpcSprite::left_pov_golems() {
    std::vector<SDL_FRect> frames1;
    std::vector<SDL_FRect> frames2;
    
    frames1.push_back({21.0f, 327.0f, 121.0f, 156.0f});
    frames1.push_back({163.0f, 327.0f, 151.0f, 156.0f});
    frames1.push_back({641.0f, 327.0f, 155.0f, 156.0f});
    
    frames2.push_back({6.0f, 420.0f, 142.0f, 159.0f});
    frames2.push_back({157.0f, 420.0f, 170.0f, 159.0f});
    frames2.push_back({636.0f, 420.0f, 167.0f, 159.0f});
    
    left_povs["Golem1"] = frames1;
    left_povs["Golem2"] = frames1;
    left_povs["Golem3"] = frames2;
}

void NpcSprite::npcs_back_povs() {
    back_pov_goblin();
    back_pov_zombie();
    back_pov_orc();
    back_pov_spiders();
    back_pov_skeletons();
    back_pov_golems();
}

void NpcSprite::npcs_front_povs() {
    front_pov_goblin();
    front_pov_zombie();
    front_pov_orc();
    front_pov_spiders();
    front_pov_skeletons();
    front_pov_golems();
}

void NpcSprite::npcs_right_povs() {
    right_pov_goblin();
    right_pov_zombie();
    right_pov_orc();
    right_pov_spiders();
    right_pov_skeletons();
    right_pov_golems();
}

void NpcSprite::npcs_left_povs() {
    left_pov_goblin();
    left_pov_zombie();
    left_pov_orc();
    left_pov_spiders();
    left_pov_skeletons();
    left_pov_golems();
}
