#include "displayPlayer.h"
#include <SDL3_image/SDL_image.h>
#include <stdexcept>
#include <string>
#include <iostream>

PlayerDisplay::PlayerDisplay(SDL_Renderer* renderer, const std::string& imagePath)
    : renderer(renderer), image(nullptr), rect{64.0f, 64.0f, 64.0f, 64.0f},
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
      keystate(other.keystate) {
    other.image = nullptr;
}

PlayerDisplay& PlayerDisplay::operator=(PlayerDisplay&& other) noexcept {
    if (this != &other) {
        if (image) SDL_DestroyTexture(image);
        renderer = other.renderer;
        image = other.image;
        rect = other.rect;
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

void PlayerDisplay::draw() const {
    SDL_RenderTexture(renderer, image, nullptr, &rect);
}
