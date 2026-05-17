#ifndef DISPLAY_PLAYER_H
#define DISPLAY_PLAYER_H

#include <SDL3/SDL.h>
#include <string>

#define PLAYER_VEL 3.0f

class PlayerDisplay {
private:
    SDL_Renderer* renderer;
    SDL_Texture* image;
    SDL_FRect rect;
    const bool* keystate;

public:
    // Carga la imagen y setea una posicion inicial
    PlayerDisplay(SDL_Renderer* renderer, const std::string& imagePath);
    ~PlayerDisplay();

    PlayerDisplay(const PlayerDisplay&) = delete;
    PlayerDisplay& operator=(const PlayerDisplay&) = delete;
    PlayerDisplay(PlayerDisplay&& other) noexcept;
    PlayerDisplay& operator=(PlayerDisplay&& other) noexcept;

    // Lee el teclado y mueve el jugador. (Solo para el jugador local)
    void update();
    // For remote players: set position from server messages
    // Para jugadores remotos: setear la posicion segun los mensajes del server
    void setPosition(float x, float y);
    void draw() const;

    void move_up();
    void move_down();
    void move_left();
    void move_right();
};

#endif
