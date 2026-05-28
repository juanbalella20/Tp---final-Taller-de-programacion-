#include "displayPlayer.h"
#include <SDL3_image/SDL_image.h>
#include "../common/game_constants.h"
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
}

PlayerDisplay::~PlayerDisplay() {
    if (image) {
        SDL_DestroyTexture(image);
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

SDL_FRect PlayerDisplay::back_pov() const {
    return {339.0f, 50.0f, 23.0f, 44.0f};
}

void PlayerDisplay::draw(const Camera& camera) const {
    SDL_FRect dst{
        camera.world_to_screen_x(rect.x),
        camera.world_to_screen_y(rect.y),
        rect.w,
        rect.h
    };
    SDL_FRect crop = back_pov();
    SDL_RenderTexture(renderer, image, &crop, &dst);
}

int PlayerDisplay::get_x() {
    return rect.x;
}

int PlayerDisplay::get_y() {
    return rect.y;
}