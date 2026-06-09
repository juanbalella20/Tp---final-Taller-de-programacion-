#ifndef NPC_SPRITE_H
#define NPC_SPRITE_H

#include <SDL3/SDL.h>
#include <vector>
#include <map>
#include <string>
#include "worldEntity.h"

// Sprite para dibujar un NPC en una celda del mapa
class NpcSprite : public WorldEntity {
    SDL_Renderer* renderer;
    SDL_Texture* texture;
    int tile_x;
    int tile_y;
    int tile_size;

    int walk_frame = 0;
    std::map<std::string, std::vector<SDL_FRect>> back_povs;
    std::map<std::string, std::vector<SDL_FRect>> front_povs;
    std::map<std::string, std::vector<SDL_FRect>> right_povs;
    std::map<std::string, std::vector<SDL_FRect>> left_povs;

    SDL_FRect current_frame(const std::vector<SDL_FRect>& frames);

    SDL_FRect npc_back_pov(const std::string& name);
    SDL_FRect npc_front_pov(const std::string& name);
    SDL_FRect npc_right_pov(const std::string& name);
    SDL_FRect npc_left_pov(const std::string& name);

    void npcs_back_povs();
    void npcs_front_povs();
    void npcs_right_povs();
    void npcs_left_povs();

    void back_pov_goblin();
    void back_pov_zombie();
    void back_pov_spiders();
    void back_pov_skeletons();
    void back_pov_golems();

    void front_pov_goblin();
    void front_pov_zombie();
    void front_pov_spiders();
    void front_pov_skeletons();
    void front_pov_golems();

    void right_pov_goblin();
    void right_pov_zombie();
    void right_pov_spiders();
    void right_pov_skeletons();
    void right_pov_golems();

    void left_pov_goblin();
    void left_pov_zombie();
    void left_pov_spiders();
    void left_pov_skeletons();
    void left_pov_golems();

public:
    NpcSprite(SDL_Renderer* renderer, SDL_Texture* texture,
              int tile_x, int tile_y, int tile_size);

    void draw(const Camera& camera, SDL_FRect src_crop) override;

    SDL_FRect back_pov(const std::string& npc_name);
    SDL_FRect front_pov(const std::string& npc_name);
    SDL_FRect right_pov(const std::string& npc_name);
    SDL_FRect left_pov(const std::string& npc_name);
};

#endif
