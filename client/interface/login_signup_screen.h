#ifndef LOGIN_SIGNUP_SCREEN_H
#define LOGIN_SIGNUP_SCREEN_H

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

#include <string>

#include "Screen.h"
#include "auth_session.h"
#include "text_field.h"

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
    // LOGIN_H).
    static constexpr SDL_FRect CHOICE_SIGNUP_BUTTON{ 134.0f, 180.0f, 228.0f, 55.0f };
    static constexpr SDL_FRect CHOICE_LOGIN_BUTTON{ 134.0f, 263.0f, 228.0f, 52.0f };

    // Carga una textura PNG. Lanza si no se pudo cargar.
    SDL_Texture* load_png(const char* path);
    // Entra a un formulario (login o signup): prende el text-input de la ventana
    // y enfoca el campo de nombre.
    void enter_form();
    // Ubica los campos sobre la PNG correspondiente al formulario activo.
    void apply_form_layout();
    // Vuelve a la pantalla de eleccion: apaga el text-input y desenfoca.
    void leave_form();
    // Pone el foco en un campo y lo saca del otro (un solo campo activo a la vez).
    void focus(TextField& field);
    // Intenta el login sincrono. Llena error_message_ o avanza a GAME.
    void try_login();
    // Confirma el formulario activo, usando la misma accion que Enter.
    void submit_current_form();
    // Valida el registro y avanza a creacion de personaje.
    void submit_signup();
    // Asegura que la sesion tenga un ClientProtocol conectado. Devuelve false y
    // setea error_message_ si no se pudo conectar.
    bool ensure_connected();

    // Manejo de eventos por vista.
    void handle_choice_event(const SDL_Event& event);
    void handle_form_event(const SDL_Event& event);
};

#endif  // LOGIN_SIGNUP_SCREEN_H
