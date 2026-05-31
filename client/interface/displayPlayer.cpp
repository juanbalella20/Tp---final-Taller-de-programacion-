#include "displayPlayer.h"
#include <SDL3_image/SDL_image.h>
#include "../common/constants/game_constants.h"
#include <stdexcept>
#include <string>
#include <iostream>

PlayerDisplay::PlayerDisplay(SDL_Renderer* renderer, const std::string& imagePath, int tileSize)
    : renderer(renderer), image(nullptr),
      rect{0.0f, 0.0f, static_cast<float>(tileSize), static_cast<float>(tileSize)},
      tileSize(tileSize),
      keystate(SDL_GetKeyboardState(nullptr)) {

    SDL_Surface* surf = IMG_Load(imagePath.c_str());
    if (!surf) {
        throw std::runtime_error(std::string("Loading player surface: ") + SDL_GetError());
    }
    image = SDL_CreateTextureFromSurface(renderer, surf);
    SDL_DestroySurface(surf);
    if (!image) {
        throw std::runtime_error(std::string("Creating player texture: ") + SDL_GetError());
    }
    SDL_SetTextureBlendMode(image, SDL_BLENDMODE_BLEND);
    
    SDL_Surface* item_surf = IMG_Load("Recursos/Graficos/101.png");
    if (!item_surf) {
        throw std::runtime_error(std::string("Loading weapon surface: ") + SDL_GetError());
    }
    weapon_image = SDL_CreateTextureFromSurface(renderer, item_surf);
    SDL_DestroySurface(item_surf);
    if (!weapon_image) {
        throw std::runtime_error(std::string("Creating weapon texture: ") + SDL_GetError());
    }
}

PlayerDisplay::~PlayerDisplay() {
    if (image) {
        SDL_DestroyTexture(image);
    }
    if (weapon_image) {
        SDL_DestroyTexture(weapon_image);
    }
}

PlayerDisplay::PlayerDisplay(PlayerDisplay&& other) noexcept
    : renderer(other.renderer), image(other.image), rect(other.rect),
      tileSize(other.tileSize), keystate(other.keystate) {
    other.image = nullptr;
}

PlayerDisplay& PlayerDisplay::operator=(PlayerDisplay&& other) noexcept {
    if (this != &other) {
        if (image) SDL_DestroyTexture(image);
        renderer = other.renderer;
        image = other.image;
        rect = other.rect;
        tileSize = other.tileSize;
        keystate = other.keystate;
        other.image = nullptr;
    }
    return *this;
}

void PlayerDisplay::move_up() {
    rect.y -= PLAYER_VEL;
}
void PlayerDisplay::move_down() {
    rect.y += PLAYER_VEL;

}
void PlayerDisplay::move_left() {
    rect.x -= PLAYER_VEL;
}
void PlayerDisplay::move_right() {
    rect.x += PLAYER_VEL;
}

void PlayerDisplay::update() {
    if (keystate[SDL_SCANCODE_LEFT]  || keystate[SDL_SCANCODE_A]) rect.x -= PLAYER_VEL;
    if (keystate[SDL_SCANCODE_RIGHT] || keystate[SDL_SCANCODE_D]) rect.x += PLAYER_VEL;
    if (keystate[SDL_SCANCODE_UP]    || keystate[SDL_SCANCODE_W]) rect.y -= PLAYER_VEL;
    if (keystate[SDL_SCANCODE_DOWN]  || keystate[SDL_SCANCODE_S]) rect.y += PLAYER_VEL;
}

void PlayerDisplay::setPosition(float x, float y) {
    rect.x = x;
    rect.y = y;
}

void PlayerDisplay::setTilePosition(int col, int row) {
    rect.x = static_cast<float>(col * tileSize);
    rect.y = static_cast<float>(row * tileSize);
}

int PlayerDisplay::getTileX() const {
    return static_cast<int>(rect.x) / tileSize;
}

int PlayerDisplay::getTileY() const {
    return static_cast<int>(rect.y) / tileSize;
}

void PlayerDisplay::reset_frame() {
    walk_frame = 0;
}

void PlayerDisplay::set_ghost(bool is_ghost) {
    ghost = is_ghost;
}

bool PlayerDisplay::is_ghost() const {
    return ghost;
}

void PlayerDisplay::set_equipped_weapon(bool has_weapon) {
    has_equipped_weapon = has_weapon;
}

SDL_FRect PlayerDisplay::back_pov() {
    static const SDL_FRect frames[] = {
        {255.0f, 51.0f, 30.0f, 40.0f},
        {285.0f, 51.0f, 30.0f, 40.0f},
        {310.0f, 51.0f, 30.0f, 40.0f},
        {335.0f, 51.0f, 30.0f, 40.0f},
        {365.0f, 51.0f, 30.0f, 40.0f},
        {394.0f, 51.0f, 30.0f, 40.0f}
    };

    if(has_equipped_weapon) {
        static const float dx[] = { 0.1f, 0.2f, 0.1f, 0.2f, 0.1f, 0.1f };
        static const float dy[] = { 0.1f, 0.1f, 0.1f, 0.1f, 0.0f, 0.0f };
        weapon_dx = dx[walk_frame % 5];
        weapon_dy = dy[walk_frame % 5];
    }

    SDL_FRect frame = frames[walk_frame % 5];
    walk_frame = (walk_frame + 1) % 5;
    return frame;
}

SDL_FRect PlayerDisplay::front_pov() {
    static const SDL_FRect frames[] = {
        {255.0f, 5.0f, 30.0f, 40.0f},
        {285.0f, 5.0f, 30.0f, 40.0f},
        {310.0f, 5.0f, 30.0f, 40.0f},
        {335.0f, 5.0f, 30.0f, 40.0f},
        {365.0f, 5.0f, 30.0f, 40.0f},
        {394.0f, 5.0f, 30.0f, 40.0f}
    };
    if(has_equipped_weapon) {
        static const float dx[] = { 0.1f, 0.1f, 0.2f, 0.2f, 0.1f, 0.1f };
        static const float dy[] = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
        weapon_dx = dx[walk_frame % 5];
        weapon_dy = dy[walk_frame % 5];
    }

    SDL_FRect frame = frames[walk_frame % 5];
    walk_frame = (walk_frame + 1) % 5;
    return frame;
}

SDL_FRect PlayerDisplay::right_pov() {
    static const SDL_FRect frames[] = {
        {259.0f, 147.0f, 30.0f, 40.0f},
        {278.0f, 147.0f, 30.0f, 40.0},
        {312.0f, 147.0f, 30.0f, 40.0},
        {340.0f, 147.0f, 30.0f, 40.0},
        {370.0f, 147.0f, 30.0f, 40.0}
    };

    if(has_equipped_weapon) {
        static const float dx[] = { 0.2f, 0.5f, 0.2f, 0.2f, 0.2f };
        static const float dy[] = { 0.1f, 0.1f, 0.1f, 0.1f, 0.0f };
        weapon_dx = dx[walk_frame % 5];
        weapon_dy = dy[walk_frame % 5];
    }

    SDL_FRect frame = frames[walk_frame % 5];
    walk_frame = (walk_frame + 1) % 5;
    return frame;
}

SDL_FRect PlayerDisplay::left_pov() {
    static const SDL_FRect frames[] = {
        {252.0f, 100.0f, 30.0f, 40.0f},
        {278.0f, 100.0f, 30.0f, 40.0},
        {309.0f, 100.0f, 30.0f, 40.0},
        {336.0f, 100.0f, 30.0f, 40.0},
        {370.0f, 100.0f, 30.0f, 40.0}
    };
    if(has_equipped_weapon) {
        static const float dx[] = { 0.5f, 0.5f, 0.4f, 0.4f, 0.1f };
        static const float dy[] = { 0.1f, 0.1f, 0.1f, 0.1f, 0.0f };
        weapon_dx = dx[walk_frame % 5];
        weapon_dy = dy[walk_frame % 5];
    }

    SDL_FRect frame = frames[walk_frame % 5];
    walk_frame = (walk_frame + 1) % 5;
    return frame;
}

void PlayerDisplay::draw(const Camera& camera, SDL_FRect crop_pov) const {
    if (ghost) {
        // Más translucidez: menor valor = más transparente
        SDL_SetTextureAlphaMod(image, 140);
    } else {
        SDL_SetTextureAlphaMod(image, 255);
    }

    SDL_FRect dst{
        camera.world_to_screen_x(rect.x),
        camera.world_to_screen_y(rect.y),
        rect.w,
        rect.h
    };
    SDL_RenderTexture(renderer, image, &crop_pov, &dst);
    if (has_equipped_weapon) {
        SDL_FRect crop = {224.0f, 96.0f, 30.0f, 30.0f};;
        float weapon_size = rect.w * 0.5f;
        SDL_FRect weapon_dst = {
            camera.world_to_screen_x(rect.x) + rect.w * weapon_dx,
            camera.world_to_screen_y(rect.y) + rect.h * weapon_dy,
            weapon_size,
            weapon_size
        };
        SDL_RenderTexture(renderer, weapon_image, &crop, &weapon_dst);
    }
}

int PlayerDisplay::get_x() {
    return rect.x;
}

int PlayerDisplay::get_y() {
    return rect.y;
}