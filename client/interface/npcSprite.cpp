#include "npcSprite.h"


NpcSprite::NpcSprite(SDL_Renderer* renderer, SDL_Texture* texture,
              int tile_x, int tile_y, int tile_size)
        : renderer(renderer), texture(texture),
          tile_x(tile_x), tile_y(tile_y), tile_size(tile_size) {}

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

void ClientGUI::crop_goblin() {
    SDL_FRect frames[] = {
        {0.0f, 0.0f, 32.0f, 32.0f},
        {285.0f, 5.0f, 30.0f, 40.0f},
        {310.0f, 5.0f, 30.0f, 40.0f},
        {335.0f, 5.0f, 30.0f, 40.0f},
        {365.0f, 5.0f, 30.0f, 40.0f},
        {394.0f, 5.0f, 30.0f, 40.0f}
    };

    enemies_crops["Goblin"] = frames;
}

void ClientGUI::crop_enemies() {

    enemies_crops["Spider1"] = {0.0f, 0.0f, 64.0f, 64.0f};
    enemies_crops["Spider2"] = {0.0f, 14.0f, 95.0f, 65.0f};
    enemies_crops["Spider3"] = {20.0f, 69.0f, 84.0f, 57.0f};
}