#ifndef DISPLAY_PLAYER_H
#define DISPLAY_PLAYER_H

#include <SDL3/SDL.h>
#include <string>
#include "worldEntity.h"

// #define PLAYER_VEL 3.0f
// para testing:
#define PLAYER_VEL 8.0f

class PlayerDisplay : public WorldEntity {
private:
    SDL_Renderer* renderer;
    SDL_Texture* image;
    SDL_FRect rect;
    int tileSize;
    const bool* keystate;  

public:
    // Carga la imagen. La posicion inicial es (0, 0). Tile size en pixels.
    PlayerDisplay(SDL_Renderer* renderer, const std::string& imagePath, int tileSize);
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
    void draw(const Camera& camera) const override;

    void move_up();
    void move_down();
    void move_left();
    void move_right();
    int get_x();
    int get_y();

};

#endif
