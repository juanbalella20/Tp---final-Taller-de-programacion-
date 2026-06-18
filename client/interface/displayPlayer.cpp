#include "displayPlayer.h"
#include <SDL3_image/SDL_image.h>
#include "../common/constants/game_constants.h"
#include "paths.h"
#include <stdexcept>
#include <string>
#include <iostream>

PlayerDisplay::PlayerDisplay(SDL_Renderer* renderer, const std::string& imagePath, int tileSize,
    const std::string& race)
    : renderer(renderer),
      rect{0.0f, 0.0f, static_cast<float>(tileSize), static_cast<float>(tileSize)},
      head_pov{0.0f, 0.0f, 0.0f, 0.0f},
      texture_loader(renderer),
      tileSize(tileSize),
      keystate(SDL_GetKeyboardState(nullptr)),
      race(race) {
    const float size_scale = (race == "dwarf" || race == "gnome") ? 0.60f : 0.80f;
    rect.w *= size_scale;
    rect.h *= size_scale;

    texture_loader.load_player(imagePath);
    texture_loader.load_ghost();

    texture_loader.load_head_for_race(race);
    texture_loader.load_items();
    equip_items_kind();
}

void PlayerDisplay::equip_items_kind() {
    // kind decide CÓMO se posiciona el sprite sobre el personaje (ver draw_equipped_item):
    // arma en la mano (animada), escudo/armadura sobre el torso, casco en la
    // cabeza.
    struct Def { const char* id; EquipKind kind; };
    const Def defs[] = {
        // Armas físicas (van en la mano, animadas con la caminata).
        {"espada", EquipKind::WEAPON},
        {"hacha",  EquipKind::WEAPON},
        {"martillo", EquipKind::WEAPON},
        {"arco_simple", EquipKind::WEAPON},
        {"arco_compuesto", EquipKind::WEAPON},
        // Varas/báculos (también en la mano).
        {"vara_fresno", EquipKind::WEAPON},
        {"baculo_nudoso", EquipKind::WEAPON},
        {"baculo_engarzado", EquipKind::WEAPON},
        {"flauta_elfica", EquipKind::WEAPON},
        // Escudos (fijos sobre el torso).
        {"escudo", EquipKind::SHIELD},
        {"escudo_tortuga", EquipKind::SHIELD},
        {"escudo_hierro", EquipKind::SHIELD},
        // Armaduras (cubren el torso/cuerpo).
        {"armadura_cuero", EquipKind::ARMOR},
        {"armadura_placas", EquipKind::ARMOR},
        {"tunica_azul", EquipKind::ARMOR},
        // Cascos (sobre la cabeza).
        {"capucha", EquipKind::HELMET},
        {"casco_hierro", EquipKind::HELMET},
        {"sombrero_magico", EquipKind::HELMET},
    };
    for (const auto& d : defs) {
        items_kind[d.id] = d.kind;
    }
}

PlayerDisplay::~PlayerDisplay() {

}

PlayerDisplay::PlayerDisplay(PlayerDisplay&& other) noexcept
    : renderer(other.renderer), rect(other.rect), head_pov(other.head_pov),
        texture_loader(other.texture_loader),
      tileSize(other.tileSize), keystate(other.keystate), race(other.race),
      items_kind(std::move(other.items_kind)),
      equipped_item_ids(std::move(other.equipped_item_ids)) {

    other.items_kind.clear();
}

PlayerDisplay& PlayerDisplay::operator=(PlayerDisplay&& other) noexcept {
    if (this != &other) {
        renderer = other.renderer;
        rect = other.rect;
        head_pov = other.head_pov;
        texture_loader = other.texture_loader;
        tileSize = other.tileSize;
        keystate = other.keystate;
        race = other.race;
        items_kind = std::move(other.items_kind);
        equipped_item_ids = std::move(other.equipped_item_ids);
        other.items_kind.clear();
    }
    return *this;
}

// void PlayerDisplay::move_up() {
//     rect.y -= PLAYER_VEL;
// }
// void PlayerDisplay::move_down() {
//     rect.y += PLAYER_VEL;
// }
// void PlayerDisplay::move_left() {
//     rect.x -= PLAYER_VEL;
// }
// void PlayerDisplay::move_right() {
//     rect.x += PLAYER_VEL;
// }

// void PlayerDisplay::update() {
//     if (keystate[SDL_SCANCODE_LEFT]  || keystate[SDL_SCANCODE_A]) rect.x -= PLAYER_VEL;
//     if (keystate[SDL_SCANCODE_RIGHT] || keystate[SDL_SCANCODE_D]) rect.x += PLAYER_VEL;
//     if (keystate[SDL_SCANCODE_UP]    || keystate[SDL_SCANCODE_W]) rect.y -= PLAYER_VEL;
//     if (keystate[SDL_SCANCODE_DOWN]  || keystate[SDL_SCANCODE_S]) rect.y += PLAYER_VEL;
// }

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
    if (ghost) {
        has_equipped_weapon = false;
        equipped_item_ids.clear();
    }
}

bool PlayerDisplay::is_ghost() const {
    return ghost;
}

void PlayerDisplay::set_equipped_weapon(bool has_weapon) {
    has_equipped_weapon = has_weapon;
    if (has_weapon) {
        reset_frame();
        int current_frame;
        switch(current_direction) {
            case ViewDirection::BACK:
                current_frame = walk_frame % 6;
                weapon_back_offset(current_frame);
                break;
            case ViewDirection::FRONT:
                current_frame = walk_frame % 6;
                weapon_front_offset(current_frame);
                break;
            case ViewDirection::LEFT:
                current_frame = walk_frame % 5;
                weapon_left_offset(current_frame);
                break;
            case ViewDirection::RIGHT:
                current_frame = walk_frame % 5;
                weapon_right_offset(current_frame);
                break;
        }
    }
}

void PlayerDisplay::set_equipped_items(const std::vector<std::string>& ids) {
    equipped_item_ids = ids;
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

SDL_FRect PlayerDisplay::back_pov(ViewDirection direction) {
    current_direction = direction;
    SDL_FRect frame;

    if (!ghost) {
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
            weapon_back_offset(current_frame);
        }

        frame = frames[current_frame];
        walk_frame = (walk_frame + 1) % 6;
    } else {
        frame = back_ghost_pov();
    }
    return frame;
}

void PlayerDisplay::weapon_back_offset(int current_frame) {
    static const float dx[] = { 0.6f, 0.5f, 0.6f, 0.6f, 0.5f, 0.5f };
    static const float dy[] = { 0.1f, 0.05f, 0.0f, 0.1f, 0.0f, 0.0f };
    weapon_dx = dx[current_frame];
    weapon_dy = dy[current_frame];
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

SDL_FRect PlayerDisplay::front_pov(ViewDirection direction) {
    current_direction = direction;
    SDL_FRect frame;

    if (!ghost) {
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
            weapon_front_offset(current_frame);
        }

        frame = frames[current_frame];
        walk_frame = (walk_frame + 1) % 6;
    } else {
        frame = front_ghost_pov();
    }
    
    return frame;
}

void PlayerDisplay::weapon_front_offset(int current_frame) {
    static const float dx[] = { -0.15f, -0.1f, 0.0f, -0.15f, -0.2f, -0.3f };
    static const float dy[] = { 0.05f, 0.05f, 0.0f, 0.05f, 0.0f, 0.0f};
    weapon_dx = dx[current_frame];
    weapon_dy = dy[current_frame];
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

SDL_FRect PlayerDisplay::right_pov(ViewDirection direction) {
    current_direction = direction;
    SDL_FRect frame;

    if (!ghost) {
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
            weapon_right_offset(current_frame);
        }

        frame = frames[current_frame];
        walk_frame = (walk_frame + 1) % 5;
    } else {
        frame = right_ghost_pov();
    }
    return frame;
}

void PlayerDisplay::weapon_right_offset(int current_frame) {
    static const float dx[] = { 0.2f, 0.5f, 0.2f, 0.2f, 0.2f };
    static const float dy[] = { 0.1f, 0.1f, 0.1f, 0.1f, 0.0f };
    weapon_dx = dx[current_frame];
    weapon_dy = dy[current_frame];
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

SDL_FRect PlayerDisplay::left_pov(ViewDirection direction) {
    current_direction = direction;
    SDL_FRect frame;

    if (!ghost) {
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
            weapon_left_offset(current_frame);
        }

        frame = frames[current_frame];
        walk_frame = (walk_frame + 1) % 5;
    } else {
        frame = left_ghost_pov();
    }
    return frame;
}

void PlayerDisplay::weapon_left_offset(int current_frame) {
    static const float dx[] = { 0.1f, 0.0f, -0.1f, -0.1f, -0.2f };
    static const float dy[] = { -0.05f, -0.05f, 0.0f, 0.0f, -0.05f };
    weapon_dx = dx[current_frame];
    weapon_dy = dy[current_frame];
}

SDL_FRect PlayerDisplay::front_ghost_pov() {
    static const SDL_FRect frames[] = {
        { 0.0f, 6.0f, 31.0f, 48.0f},
        {31.0f, 6.0f, 32.0f, 48.0},
        {63.0f, 6.0f, 32.0f, 48.0},
        {95.0f, 6.0f, 32.0f, 48.0},
        {127.0f, 6.0f, 33.0f, 48.0}
    };

    int current_frame = walk_frame % 5;

    SDL_FRect frame = frames[current_frame];
    walk_frame = (walk_frame + 1) % 5;
    return frame;
}

SDL_FRect PlayerDisplay::back_ghost_pov() {
    static const SDL_FRect frames[] = {
        { 0.0f, 71.0f, 31.0f, 45.0f},
        {31.0f, 71.0f, 32.0f, 45.0},
        {63.0f, 71.0f, 32.0f, 45.0},
        {95.0f, 71.0f, 32.0f, 45.0},
        {127.0f, 71.0f, 33.0f, 45.0}
    };

    int current_frame = walk_frame % 5;

    SDL_FRect frame = frames[current_frame];
    walk_frame = (walk_frame + 1) % 5;
    return frame;
}

SDL_FRect PlayerDisplay::left_ghost_pov() {
    static const SDL_FRect frames[] = {
        { 0.0f, 201.0f, 31.0f, 42.0f},
        {31.0f, 201.0f, 32.0f, 42.0},
        {63.0f, 201.0f, 32.0f, 42.0},
        {95.0f, 201.0f, 32.0f, 42.0},
        {127.0f, 201.0f, 33.0f, 42.0}
    };

    int current_frame = walk_frame % 5;

    SDL_FRect frame = frames[current_frame];
    walk_frame = (walk_frame + 1) % 5;
    return frame;
}

SDL_FRect PlayerDisplay::right_ghost_pov() {
    static const SDL_FRect frames[] = {
        { 0.0f, 135.0f, 31.0f, 43.0f},
        {31.0f, 135.0f, 32.0f, 43.0},
        {63.0f, 135.0f, 32.0f, 43.0},
        {95.0f, 135.0f, 32.0f, 43.0},
        {127.0f, 135.0f, 33.0f, 43.0}
    };

    int current_frame = walk_frame % 5;

    SDL_FRect frame = frames[current_frame];
    walk_frame = (walk_frame + 1) % 5;
    return frame;
}


void PlayerDisplay::draw_player(const Camera& camera, SDL_FRect crop) {
    // Ancla por los pies: todos los personajes tocan el borde inferior del tile
    // independientemente de su escala (enano 0.60 vs humano 0.80).
    float foot_offset = static_cast<float>(tileSize) - rect.h;
    SDL_FRect dst {
        camera.world_to_screen_x(rect.x),
        camera.world_to_screen_y(rect.y) + foot_offset,
        rect.w,
        rect.h
    };

    SDL_Texture* current_image;
    if (ghost) {
        current_image = texture_loader.get_ghost_texture();
    } else {
        current_image = texture_loader.get_player_texture();
    }
    SDL_RenderTexture(renderer, current_image, &crop, &dst);
    if (!ghost) {
        draw_player_head(camera);
    }
}

void PlayerDisplay::draw_player_head(const Camera& camera) {
    float head_width = rect.w * 0.5f;
    float aspect_ratio = head_pov.h / head_pov.w;
    float head_height = head_width * aspect_ratio;

    float foot_offset = static_cast<float>(tileSize) - rect.h;
    float base_head_x = camera.world_to_screen_x(rect.x) + (rect.w - head_width) * 0.5f;
    float base_head_y = camera.world_to_screen_y(rect.y) + foot_offset - (rect.h * 0.25f);

    SDL_FRect head_dst = {
        base_head_x + (rect.w * head_dx),
        base_head_y + (rect.h * head_dy),
        head_width,
        head_height
    };

    SDL_Texture* head = texture_loader.get_head_texture();
    SDL_RenderTexture(renderer, head, &head_pov, &head_dst);

    draw_gnome_hat(camera, head_dst);
}

void PlayerDisplay::draw_gnome_hat(const Camera& camera, const SDL_FRect& head_dst) {
    if (race == "gnome") {
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
        SDL_Texture* hat = texture_loader.get_hat_texture();
        SDL_RenderTexture(renderer, hat, &crop, &hat_dst);
    }
}

void PlayerDisplay::draw_equipped_item(const Camera& camera, bool behind_body) {
    // Dibuja cada item equipado con su sprite real. Arma/báculo van en la mano
    // (siguen la animación de caminata con weapon_dx/dy). Si no hay sprite
    // registrado para el id, no se dibuja nada.
    //
    // behind_body distingue las dos pasadas de draw(): el arma se dibuja DETRÁS
    // del cuerpo cuando el personaje da la espalda (BACK/LEFT) y delante cuando
    // mira al frente (FRONT/RIGHT).
    for (const auto& id : equipped_item_ids) {
        auto it = items_kind.find(id);
        if (it == items_kind.end()) continue;
        EquipKind kind = it->second;

        // Escudo, armadura y casco NO se dibujan sobre el personaje: su estado
        // (equipado o no) se muestra en el panel del HUD (drawEquipStatus). Sobre
        // el jugador solo se ve el arma/báculo (en la mano).
        if (kind == EquipKind::SHIELD || kind == EquipKind::ARMOR ||
            kind == EquipKind::HELMET) {
            continue;
        }

        // Filtrado por pasada: el arma va detrás del cuerpo cuando el
        // personaje da la espalda (BACK/LEFT) y delante en FRONT/RIGHT.
        bool item_behind = (current_direction == ViewDirection::BACK ||
                            current_direction == ViewDirection::LEFT);
        if (item_behind != behind_body) continue;

        float off_x, off_y, size;
        double angle = 0.0;
        SDL_FlipMode flip = SDL_FLIP_NONE;

        switch (kind) {
            case EquipKind::WEAPON:
                // Arma/báculo: en la mano, animado con la caminata. Se rota 270°
                // mirando al frente/izquierda para que apunte hacia adelante.
                size = rect.w * 0.5f;
                off_x = weapon_dx;
                off_y = weapon_dy;
                if (current_direction == ViewDirection::FRONT ||
                    current_direction == ViewDirection::LEFT) {
                    angle = 270.0;
                }
                break;

            case EquipKind::ARMOR:
                // Armadura: cubre el torso/cuerpo. Centrada sobre el cuerpo y del
                // mismo tamaño, en todas las direcciones (se ve similar de frente
                // y de espalda).
                size = rect.w * 0.85f;
                off_x = (1.0f - 0.85f) / 2.0f;  // centrada horizontalmente
                off_y = 0.20f;                  // un poco por debajo de la cabeza
                break;

            case EquipKind::HELMET:
            default:
                // Casco: sobre la cabeza. La cabeza se dibuja por encima del cuerpo
                // (rect.y es el top del cuerpo), así que el casco va con offset_y
                // negativo para subir hasta la cabeza.
                size = rect.w * 0.55f;
                off_x = (1.0f - 0.55f) / 2.0f;  // centrado horizontalmente
                off_y = -0.45f;                 // sobre la cabeza
                break;
        }

        float foot_offset = static_cast<float>(tileSize) - rect.h;
        SDL_FRect dst = {
            camera.world_to_screen_x(rect.x) + rect.w * off_x,
            camera.world_to_screen_y(rect.y) + foot_offset + rect.h * off_y,
            size,
            size
        };
        
        SDL_Texture* tex = texture_loader.get_texture_of_item(id);
        SDL_RenderTextureRotated(renderer, tex, nullptr, &dst, angle, nullptr, flip);
    }
}

void PlayerDisplay::draw(const Camera& camera, SDL_FRect body_pov) {
    if (!ghost) {
        // Pasada 1: items que van DETRÁS del cuerpo (armas/escudo cuando el personaje
        // da la espalda). draw_equipped_item filtra cuáles corresponden a esta pasada.
        draw_equipped_item(camera, /*behind_body=*/true);
    }

    draw_player(camera, body_pov);

    if (!ghost) {
        // Pasada 2: items que van DELANTE del cuerpo (armas/escudo de frente, y
        // siempre la armadura y el casco).
        draw_equipped_item(camera, /*behind_body=*/false);
    }
}

int PlayerDisplay::get_x() {
    return rect.x;
}

int PlayerDisplay::get_y() {
    return rect.y;
}
