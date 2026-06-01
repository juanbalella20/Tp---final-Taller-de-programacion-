#include "displayPlayer.h"
#include <SDL3_image/SDL_image.h>
#include "../common/constants/game_constants.h"
#include <stdexcept>
#include <string>
#include <iostream>

PlayerDisplay::PlayerDisplay(SDL_Renderer* renderer, const std::string& imagePath, int tileSize,
    const std::string& race)
    : renderer(renderer), image(nullptr),
      rect{0.0f, 0.0f, static_cast<float>(tileSize), static_cast<float>(tileSize)},
      head_pov{0.0f, 0.0f, 0.0f, 0.0f},
      tileSize(tileSize),
      keystate(SDL_GetKeyboardState(nullptr)),
      race(race) {
    const float size_scale = (race == "dwarf" || race == "gnome") ? 0.75f : 1.0f;
    rect.w *= size_scale;
    rect.h *= size_scale;

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
    
    SDL_Surface* item_surf = IMG_Load("imagenes/101.png");
    if (!item_surf) {
        throw std::runtime_error(std::string("Loading weapon surface: ") + SDL_GetError());
    }
    weapon_image = SDL_CreateTextureFromSurface(renderer, item_surf);
    SDL_DestroySurface(item_surf);
    if (!weapon_image) {
        throw std::runtime_error(std::string("Creating weapon texture: ") + SDL_GetError());
    }

    load_heads();
}

PlayerDisplay::~PlayerDisplay() {
    if (image) {
        SDL_DestroyTexture(image);
    }
    if (weapon_image) {
        SDL_DestroyTexture(weapon_image);
    }
    if (head_image) {
        SDL_DestroyTexture(head_image);
    }
    if (hat_image) {
        SDL_DestroyTexture(hat_image);
    }
}

PlayerDisplay::PlayerDisplay(PlayerDisplay&& other) noexcept
    : renderer(other.renderer), image(other.image), weapon_image(other.weapon_image),
      head_image(other.head_image), hat_image(other.hat_image), rect(other.rect), head_pov(other.head_pov),
      tileSize(other.tileSize), keystate(other.keystate), race(other.race) {
    other.image = nullptr;
    other.weapon_image = nullptr;
    other.head_image = nullptr;
    other.hat_image = nullptr;
}

PlayerDisplay& PlayerDisplay::operator=(PlayerDisplay&& other) noexcept {
    if (this != &other) {
        if (image) SDL_DestroyTexture(image);
        if (weapon_image) SDL_DestroyTexture(weapon_image);
        if (head_image) SDL_DestroyTexture(head_image);
        if (hat_image) SDL_DestroyTexture(hat_image);
        renderer = other.renderer;
        image = other.image;
        weapon_image = other.weapon_image;
        head_image = other.head_image;
        hat_image = other.hat_image;
        rect = other.rect;
        head_pov = other.head_pov;
        tileSize = other.tileSize;
        keystate = other.keystate;
        race = other.race;
        other.image = nullptr;
        other.weapon_image = nullptr;
        other.head_image = nullptr;
        other.hat_image = nullptr;
    }
    return *this;
}

void PlayerDisplay::load_heads() {
    SDL_Surface* head_surf;
    SDL_Surface* hat_surf;

    if (race == "human") {
        head_surf = IMG_Load("imagenes/420.png");
    } else if (race == "elf") {
        head_surf = IMG_Load("imagenes/422.png");
    } else if (race == "dwarf") {
        head_surf = IMG_Load("imagenes/426.png");
    } else if (race == "gnome") {
        head_surf = IMG_Load("imagenes/426.png");
        hat_surf = IMG_Load("imagenes/437.png");
    }

    if (!head_surf) {
        throw std::runtime_error(std::string("Loading head surface: ") + SDL_GetError());
    }
    if (!hat_surf) {
        throw std::runtime_error(std::string("Loading hat surface: ") + SDL_GetError());
    }
    head_image = SDL_CreateTextureFromSurface(renderer, head_surf);
    SDL_DestroySurface(head_surf);
    hat_image = SDL_CreateTextureFromSurface(renderer, hat_surf);
    SDL_DestroySurface(hat_surf);
    if (!head_image) {
        throw std::runtime_error(std::string("Creating head texture: ") + SDL_GetError());
    }
    if (!hat_image) {
        throw std::runtime_error(std::string("Creating hat texture: ") + SDL_GetError());
    }
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

void PlayerDisplay::head_back_pov() {
    if (race == "human") {
        head_pov = { 408.0f, 332.0f, 19.0f, 16.0f };
    } else if (race == "elf") {
        head_pov = { 58.0f, 77.0f, 18.0f, 15.0f };
    } else if (race == "dwarf") {
        head_pov = { 407.0f, 79.0f, 21.0f, 17.0f };
    } else if (race == "gnome") {
        head_pov = {328.0f, 81.0f, 17.0f, 15.0f};
        hat_dy = -0.8f;
    }
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

    int current_frame = walk_frame % 6;

    static const float h_dx[] = { -0.05f, -0.15f, -0.05f, 0.0f, -0.1f, -0.15f };
    static const float h_dy[] = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };
    head_dx = h_dx[current_frame];
    head_dy = h_dy[current_frame];

    head_back_pov();

    if(has_equipped_weapon) {
        static const float dx[] = { 0.1f, 0.2f, 0.1f, 0.2f, 0.1f, 0.1f };
        static const float dy[] = { 0.1f, 0.1f, 0.1f, 0.1f, 0.0f, 0.0f };
        weapon_dx = dx[current_frame];
        weapon_dy = dy[current_frame];
    }

    SDL_FRect frame = frames[current_frame];
    walk_frame = (walk_frame + 1) % 6;
    return frame;
}

void PlayerDisplay::head_front_pov() {
    if (race == "human") {
        head_pov = { 408.0f, 267.0f, 19.0f, 19.0f };
    } else if (race == "elf") {
        head_pov = { 58.0f, 14.0f, 18.0f, 14.0f };
    } else if (race == "dwarf") {
        head_pov = { 407.0f, 16.0f, 19.0f, 16.0f };
    } else if (race == "gnome") {
        head_pov = {328.0f, 17.0f, 17.0f, 13.0f};
        hat_dx = 0.05f;
        hat_dy = -1.0f;
    }
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

    int current_frame = walk_frame % 6;

    static const float h_dx[] = { -0.05f, -0.15f, -0.05f, 0.0f, -0.1f, -0.2f };
    static const float h_dy[] = { -0.1f, -0.1f, -0.1f, -0.1f, -0.1f, -0.1f };
    head_dx = h_dx[current_frame];
    head_dy = h_dy[current_frame];

    head_front_pov();

    if(has_equipped_weapon) {
        static const float dx[] = { 0.1f, 0.1f, 0.2f, 0.2f, 0.1f, 0.1f };
        static const float dy[] = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
        weapon_dx = dx[current_frame];
        weapon_dy = dy[current_frame];
    }

    SDL_FRect frame = frames[current_frame];
    walk_frame = (walk_frame + 1) % 6;
    return frame;
}

void PlayerDisplay::head_right_pov() {
    if (race == "human") {
        head_pov = { 407.0f, 461.0f, 18.0f, 16.0f };
    } else if (race == "elf") {
        head_pov = { 57.0f, 205.0f, 19.0f, 15.0f };
    } else if (race == "dwarf") {
        head_pov = { 410.0f, 208.0f, 19.0f, 16.0f };
    } else if (race == "gnome") {
        head_pov = {328.0f, 209.0f, 19.0f, 16.0f};
        hat_dy = -0.9f;
    }
}

SDL_FRect PlayerDisplay::right_pov() {
    static const SDL_FRect frames[] = {
        {259.0f, 147.0f, 30.0f, 40.0f},
        {278.0f, 147.0f, 30.0f, 40.0},
        {312.0f, 147.0f, 30.0f, 40.0},
        {340.0f, 147.0f, 30.0f, 40.0},
        {370.0f, 147.0f, 30.0f, 40.0}
    };

    int current_frame = walk_frame % 5;

    static const float h_dx[] = { -0.2f, 0.1f, -0.15f, -0.2f, -0.25f };
    static const float h_dy[] = { -0.1f, -0.1f, -0.1f, -0.1f, -0.1f };
    head_dx = h_dx[current_frame];
    head_dy = h_dy[current_frame];

    head_right_pov();

    if(has_equipped_weapon) {
        static const float dx[] = { 0.2f, 0.5f, 0.2f, 0.2f, 0.2f };
        static const float dy[] = { 0.1f, 0.1f, 0.1f, 0.1f, 0.0f };
        weapon_dx = dx[current_frame];
        weapon_dy = dy[current_frame];
    }

    SDL_FRect frame = frames[current_frame];
    walk_frame = (walk_frame + 1) % 5;
    return frame;
}

void PlayerDisplay::head_left_pov() {
    if (race == "human") {
        head_pov = { 407.0f, 397.0f, 18.0f, 16.0f };
    } else if (race == "elf") {
        head_pov = { 57.0f, 142.0f, 18.0f, 16.0f };
    } else if (race == "dwarf") {
        head_pov = { 405.0f, 144.0f, 18.0f, 16.0f };
    } else if (race == "gnome") {
        head_pov = {328.0f, 144.0f, 20.0f, 16.0f};
        head_dx += 0.05f;
        head_dy += 0.05f;
        hat_dy = -1.0f;
    }
}

SDL_FRect PlayerDisplay::left_pov() {
    static const SDL_FRect frames[] = {
        {252.0f, 100.0f, 30.0f, 40.0f},
        {278.0f, 100.0f, 30.0f, 40.0},
        {309.0f, 100.0f, 30.0f, 40.0},
        {336.0f, 100.0f, 30.0f, 40.0},
        {370.0f, 100.0f, 30.0f, 40.0}
    };

    int current_frame = walk_frame % 5;

    static const float h_dx[] = { 0.0f, 0.0f, -0.05f, -0.05f, -0.3f };
    static const float h_dy[] = { -0.1f, -0.1f, -0.1f, -0.1f, -0.1f };
    head_dx = h_dx[current_frame];
    head_dy = h_dy[current_frame];

    head_left_pov();

    if(has_equipped_weapon) {
        static const float dx[] = { 0.5f, 0.5f, 0.4f, 0.4f, 0.1f };
        static const float dy[] = { 0.1f, 0.1f, 0.1f, 0.1f, 0.0f };
        weapon_dx = dx[current_frame];
        weapon_dy = dy[current_frame];
    }

    SDL_FRect frame = frames[current_frame];
    walk_frame = (walk_frame + 1) % 5;
    return frame;
}

void PlayerDisplay::set_transparency(SDL_Texture* img) const {
    if (ghost) {
        SDL_SetTextureAlphaMod(img, 140);
    } else {
        SDL_SetTextureAlphaMod(img, 255);
    }
}

void PlayerDisplay::draw_player(const Camera& camera, SDL_FRect body_pov) const {
    SDL_FRect dst {
        camera.world_to_screen_x(rect.x),
        camera.world_to_screen_y(rect.y),
        rect.w,
        rect.h
    };

    SDL_RenderTexture(renderer, image, &body_pov, &dst);
}

void PlayerDisplay::draw_player_head(const Camera& camera) const {
    float head_width = rect.w * 0.5f;
    float aspect_ratio = head_pov.h / head_pov.w;
    float head_height = head_width * aspect_ratio;

    float base_head_x = camera.world_to_screen_x(rect.x) + (rect.w - head_width) * 0.5f;
    float base_head_y = camera.world_to_screen_y(rect.y) - (rect.h * 0.25f);

    SDL_FRect head_dst = {
        base_head_x + (rect.w * head_dx),
        base_head_y + (rect.h * head_dy),
        head_width,
        head_height
    };

    SDL_RenderTexture(renderer, head_image, &head_pov, &head_dst);

    draw_gnome_hat(camera, head_dst);
}

void PlayerDisplay::draw_gnome_hat(const Camera& camera, const SDL_FRect& head_dst) const {
    if (race == "gnome") {
        set_transparency(hat_image);

        SDL_FRect crop = {194.0f, 66.0f, 19.0f, 18.0f};
        float hat_width = head_dst.w * 1.1f;
        float hat_aspect_ratio = crop.h / crop.w;
        float hat_height = hat_width * hat_aspect_ratio;
        SDL_FRect hat_dst = {
            head_dst.x + (head_dst.w * hat_dx),
            head_dst.y + (head_dst.h * hat_dy),
            hat_width,
            hat_height
        };
        SDL_RenderTexture(renderer, hat_image, &crop, &hat_dst);
    }
}

void PlayerDisplay::draw_equipped_item(const Camera& camera) const {
    if (has_equipped_weapon) {
        set_transparency(weapon_image);
        SDL_FRect crop = {224.0f, 96.0f, 30.0f, 30.0f};
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

void PlayerDisplay::draw(const Camera& camera, SDL_FRect body_pov) const {
    set_transparency(image);
    set_transparency(head_image);

    draw_player(camera, body_pov);
    draw_player_head(camera);

    draw_equipped_item(camera);
}

int PlayerDisplay::get_x() {
    return rect.x;
}

int PlayerDisplay::get_y() {
    return rect.y;
}