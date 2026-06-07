#ifndef LOGIN_SIGNUP_SCREEN_H
#define LOGIN_SIGNUP_SCREEN_H

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

#include <string>

#include "Screen.h"
#include "auth_session.h"

// ============================================================================
// STUB (placeholder de Juan) — lo reemplaza la implementacion real de Agustin.
//
// El ScreenManager (parte de Juan) ya esta listo para esta pantalla; este stub
// existe solo para que la rama de Juan compile y el flujo
// WELCOME -> LOGIN_SIGNUP -> (CHARACTER_CREATION) -> GAME sea navegable y
// testeable end-to-end mientras Agustin termina su version con campos de texto,
// botones "Ingresar"/"Crear cuenta" y el auth sincrono completo.
//
// Comportamiento minimo:
//  - Tab: alterna entre name/password (escritura via SDL text input).
//  - L: intenta LOGIN sincrono con lo tipeado.
//  - C: pasa a CHARACTER_CREATION dejando name/password en la AuthSession.
//  - Esc: EXIT.
// ============================================================================
class LoginSignupScreen : public Screen {
 private:
    SDL_Renderer* renderer;   // no-owning (ScreenManager)
    SDL_Window* window;       // no-owning
    TTF_Font* font;           // no-owning (compartida)
    AuthSession& session;
    std::string host;
    std::string port;

    std::string name;
    std::string password;
    bool editing_password = false;  // foco: false=name, true=password
    std::string error_message;
    ScreenState next = ScreenState::LOGIN_SIGNUP;

    void draw_text(const char* text, float x, float y, SDL_Color color);
    bool ensure_protocol();          // crea Socket+ClientProtocol diferido
    void try_login();                // auth LOGIN sincrono

 public:
    LoginSignupScreen(SDL_Renderer* renderer, SDL_Window* window, TTF_Font* font,
                      AuthSession& session, std::string host, std::string port);
    ~LoginSignupScreen() override;

    void handleEvent(const SDL_Event& event) override;
    void update() override;
    void render() override;
    ScreenState nextState() const override;
};

#endif  // LOGIN_SIGNUP_SCREEN_H
