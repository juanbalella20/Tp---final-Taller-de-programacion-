#ifndef LOGIN_SIGNUP_SCREEN_H
#define LOGIN_SIGNUP_SCREEN_H

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

#include <string>

#include "Screen.h"
#include "auth_session.h"
#include "text_field.h"

// Pantalla de login/registro. Maneja internamente (sub-estado, sin tocar el enum
// ScreenState) tres vistas, cada una con su propia imagen PNG de fondo:
//   CHOICE      -> elegir entre Login o Signup.
//   LOGIN_FORM  -> formulario de login   -> MSG_LOGIN sincrono; si OK entra al juego.
//   SIGNUP_FORM -> formulario de registro -> deja datos en la sesion y pasa a
//                  CHARACTER_CREATION (el MSG_REGISTER se manda desde alli).
//
// La conexion al socket esta DIFERIDA: se crea el ClientProtocol en la sesion
// recien al primer intento de login. Ante MSG_AUTH_ERROR se muestra el motivo
// (viene en chat_content) y se mantiene al usuario en la pantalla.
//
// No es duena del window/renderer/font: los recibe del ScreenManager. El
// ScreenManager hace el SDL_RenderPresent; esta pantalla solo dibuja. Libera sus
// texturas propias en el destructor.
class LoginSignupScreen : public Screen {
 public:
    LoginSignupScreen(SDL_Renderer* renderer, SDL_Window* window, TTF_Font* font,
                      AuthSession& session, std::string host, std::string port);
    ~LoginSignupScreen() override;

    void handleEvent(const SDL_Event& event) override;
    void update() override;
    void render() override;
    ScreenState nextState() const override;

 private:
    // Sub-estado interno (cual de las tres vistas se muestra).
    enum class View { CHOICE, LOGIN_FORM, SIGNUP_FORM };

    // Rutas de las imagenes PNG (relativas al cwd, resueltas con paths::asset).
    // TODO: cambiar por los nombres reales de tus PNGs.
    static constexpr char CHOICE_PNG[] = "imagenes/login/choice.png";
    static constexpr char LOGIN_PNG[]  = "imagenes/login/login.png";
    static constexpr char SIGNUP_PNG[] = "imagenes/login/signup.png";

    // Presentacion logica de esta pantalla (igual que el launcher para no saltar).
    static constexpr int LOGIN_W = 493;
    static constexpr int LOGIN_H = 479;

    SDL_Renderer* renderer_;   // no-owning (ScreenManager)
    SDL_Window* window_;       // no-owning
    TTF_Font* font_;           // no-owning (compartida)
    AuthSession& session_;
    std::string host_;
    std::string port_;

    // Texturas de fondo, una por vista (propias: liberar en el destructor).
    SDL_Texture* choice_bg_ = nullptr;
    SDL_Texture* login_bg_ = nullptr;
    SDL_Texture* signup_bg_ = nullptr;

    View view_ = View::CHOICE;

    TextField name_field_;
    TextField password_field_;
    std::string error_message_;
    ScreenState next_ = ScreenState::LOGIN_SIGNUP;

    // Zonas clickeables sobre la PNG de eleccion (coordenadas logicas LOGIN_W x
    // LOGIN_H). TODO: ajustar a la imagen real.
    static constexpr SDL_FRect CHOICE_LOGIN_BUTTON{ 96.0f, 200.0f, 300.0f, 50.0f };
    static constexpr SDL_FRect CHOICE_SIGNUP_BUTTON{ 96.0f, 280.0f, 300.0f, 50.0f };

    // Carga una textura PNG. Lanza si no se pudo cargar.
    SDL_Texture* load_png(const char* path);
    // Pone el foco en un campo y lo saca del otro (un solo campo activo a la vez).
    void focus(TextField& field);
    // Intenta el login sincrono. Llena error_message_ o avanza a GAME.
    void try_login();
    // Asegura que la sesion tenga un ClientProtocol conectado. Devuelve false y
    // setea error_message_ si no se pudo conectar.
    bool ensure_connected();

    // Manejo de eventos por vista.
    void handle_choice_event(const SDL_Event& event);
    void handle_form_event(const SDL_Event& event);
};

#endif  // LOGIN_SIGNUP_SCREEN_H
