#ifndef DISPLAY_PLAYER_H
#define DISPLAY_PLAYER_H

#include <SDL3/SDL.h>
#include <string>
#include <vector>
#include <map>
#include "worldEntity.h"

// #define PLAYER_VEL 3.0f
// para testing:
#define PLAYER_VEL 8.0f

class PlayerDisplay : public WorldEntity {
private:
    SDL_Renderer* renderer;
    SDL_Texture* image;
    SDL_Texture* weapon_image;
    SDL_Texture* head_image;
    SDL_Texture* hat_image;
    SDL_FRect rect;
    SDL_FRect head_pov;
    int tileSize;
    const bool* keystate;
    bool ghost = false;
    std::string race;

    float weapon_dx = 0.0f;
    float weapon_dy = 0.0f;
    float head_dx = 0.0f;
    float head_dy = 0.0f;
    float hat_dx = 0.0f;
    float hat_dy = 0.0f;
    int walk_frame = 0;
    bool has_equipped_weapon = false;

    // Sprite de cada item equipable, por id (textura + recorte en su spritesheet).
    struct EquipSprite {
        SDL_Texture* texture;
        SDL_FRect crop;
    };
    std::map<std::string, EquipSprite> equip_sprites;
    // Ids de los items actualmente equipados, para dibujarlos sobre el jugador.
    std::vector<std::string> equipped_item_ids;

    void load_heads();
    void load_equip_sprites();

    void head_back_pov();
    void head_front_pov();
    void head_right_pov();
    void head_left_pov();

    void set_transparency(SDL_Texture* img) const;
    void draw_player(const Camera& camera, SDL_FRect body_pov) const;
    void draw_player_head(const Camera& camera) const;
    void draw_gnome_hat(const Camera& camera, const SDL_FRect& head_dst) const;
    void draw_equipped_item(const Camera& camera) const;

public:
    // Carga la imagen. La posicion inicial es (0, 0). Tile size en pixels.
    PlayerDisplay(SDL_Renderer* renderer, const std::string& imagePath, int tileSize, const std::string& race);
    ~PlayerDisplay();

    PlayerDisplay(const PlayerDisplay&) = delete;
    PlayerDisplay& operator=(const PlayerDisplay&) = delete;
    PlayerDisplay(PlayerDisplay&& other) noexcept;
    PlayerDisplay& operator=(PlayerDisplay&& other) noexcept;

    // Lee el teclado y mueve el jugador. (Solo para el jugador local)
    void update();
    // Setea la posicion segun coords en celdas (las que manda el server).
    void setTilePosition(int col, int row);
    // Posicion actual en celdas (derivada de rect / tileSize).
    int getTileX() const;
    int getTileY() const;
    // Setea la posicion en pixels (legacy; preferir setTilePosition).
    void setPosition(float x, float y);
    void set_ghost(bool ghost);
    bool is_ghost() const;
    void set_equipped_weapon(bool has_weapon);
    // Setea la lista de ids equipados (arma/báculo + defensas) para que el sprite
    // del jugador muestre cada item con su gráfico correcto.
    void set_equipped_items(const std::vector<std::string>& ids);
    void draw(const Camera& camera, SDL_FRect crop_pov) const override;
    SDL_FRect right_pov();
    SDL_FRect left_pov();
    SDL_FRect front_pov();
    SDL_FRect back_pov();

    void move_up();
    void move_down();
    void move_left();
    void move_right();
    int get_x();
    int get_y();

    void reset_frame();
};

#endif
