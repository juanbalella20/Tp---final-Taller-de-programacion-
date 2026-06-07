#ifndef CHARACTER_CREATION_SCREEN_H
#define CHARACTER_CREATION_SCREEN_H

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

#include <string>

#include "Screen.h"
#include "auth_session.h"

// ============================================================================
// STUB (placeholder de Juan) — lo reemplaza la implementacion real de Agustin.
//
// Se entra solo desde "Crear cuenta" (LOGIN_SIGNUP). Lee name/password de la
// AuthSession. Comportamiento minimo:
//  - Flechas izq/der: cambiar raza.  Flechas arriba/abajo: cambiar clase.
//  - Enter: REGISTER sincrono con la raza/clase elegidas.
//  - Esc: volver a LOGIN_SIGNUP.
// ============================================================================
class CharacterCreationScreen : public Screen {
 private:
    SDL_Renderer* renderer;   // no-owning
    SDL_Window* window;       // no-owning
    TTF_Font* font;           // no-owning
    AuthSession& session;
    std::string host;
    std::string port;

    int selected_race = 0;
    int selected_class = 0;
    std::string error_message;
    ScreenState next = ScreenState::CHARACTER_CREATION;

    void draw_text(const char* text, float x, float y, SDL_Color color);
    bool ensure_protocol();
    void try_register();

 public:
    CharacterCreationScreen(SDL_Renderer* renderer, SDL_Window* window, TTF_Font* font,
                            AuthSession& session, std::string host, std::string port);
    ~CharacterCreationScreen() override = default;

    void handleEvent(const SDL_Event& event) override;
    void update() override;
    void render() override;
    ScreenState nextState() const override;
};

#endif  // CHARACTER_CREATION_SCREEN_H
