#ifndef DISPLAY_PLAYER_H
#define DISPLAY_PLAYER_H

#include <SDL3/SDL.h>
#include <string>
#include <vector>
#include <map>
#include "worldEntity.h"
#include "texture_loader.h"

enum class ViewDirection {
    BACK = 0,    // Norte
    FRONT = 1,   // Sur
    RIGHT = 2,   // Este
    LEFT = 3     // Oeste
};

class PlayerDisplay : public WorldEntity {
private:
    SDL_Renderer* renderer;
    SDL_FRect rect;
    SDL_FRect head_pov;
    TextureLoader texture_loader;
    int tileSize;

    int tile_col = 0;
    int tile_row = 0;
    static constexpr float SNAP_TILES = 3.0f;
    static constexpr float SMOOTH = 0.25f;
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

    ViewDirection current_direction = ViewDirection::FRONT;
    // Categoría de item equipable: decide CÓMO se posiciona el sprite sobre el
    // personaje (mano animada, torso fijo, cabeza). Ver draw_equipped_item.
    enum class EquipKind { WEAPON, SHIELD, ARMOR, HELMET };

    std::map<std::string, EquipKind> items_kind;
    // Ids de los items actualmente equipados, para dibujarlos sobre el jugador.
    std::vector<std::string> equipped_item_ids;

    void load_heads();
    void equip_items_kind();

    void head_back_pov();
    void head_front_pov();
    void head_right_pov();
    void head_left_pov();

    void draw_player(const Camera& camera, SDL_FRect body_pov);
    void draw_player_head(const Camera& camera);
    void draw_gnome_hat(const Camera& camera, const SDL_FRect& head_dst);
    void draw_equipped_item(const Camera& camera, bool behind_body);

    void weapon_back_offset(int current_frame);
    void weapon_front_offset(int current_frame);
    void weapon_right_offset(int current_frame);
    void weapon_left_offset(int current_frame);

    SDL_FRect front_ghost_pov();
    SDL_FRect back_ghost_pov();
    SDL_FRect left_ghost_pov();
    SDL_FRect right_ghost_pov();

public:
    // Carga la imagen. La posicion inicial es (0, 0). Tile size en pixels.
    PlayerDisplay(SDL_Renderer* renderer, const std::string& imagePath, int tileSize, const std::string& race);
    ~PlayerDisplay();

    PlayerDisplay(const PlayerDisplay&) = delete;
    PlayerDisplay& operator=(const PlayerDisplay&) = delete;
    PlayerDisplay(PlayerDisplay&& other) noexcept;
    PlayerDisplay& operator=(PlayerDisplay&& other) noexcept;

    // Setea la posicion segun coords en celdas (las que manda el server).
    void setTilePosition(int col, int row);
    // Acerca la posicion visual (rect) hacia su tile. Se llama una vez por frame.
    void update_motion();
    // Posicion actual en celdas (derivada de rect / tileSize).
    int getTileX() const;
    int getTileY() const;
    void set_ghost(bool ghost);
    bool is_ghost() const;
    void set_equipped_weapon(bool has_weapon);
    void set_equipped_items(const std::vector<std::string>& ids);
    void draw(const Camera& camera, SDL_FRect crop) override;
    SDL_FRect right_pov(ViewDirection direction);
    SDL_FRect left_pov(ViewDirection direction);
    SDL_FRect front_pov(ViewDirection direction);
    SDL_FRect back_pov(ViewDirection direction);

    int get_x();
    int get_y();

    void reset_frame();
};

#endif
