#include "login_signup_screen.h"

#include <utility>

#include <SDL3_image/SDL_image.h>

#include "paths.h"
#include "../comunication/clientProtocol.h"
#include "../../common/socket/socket.h"
#include "../../common/commands/clientCmd.h"
#include "../../common/commands/gameMsg.h"
#include "../../common/constants/protocol_constants.h"
#include "ui_helpers.h"

namespace {
constexpr SDL_Color COLOR_ERROR{ 220, 80, 70, 255 };

// Field and button boxes are in the 493x479 logical presentation used by this screen.
constexpr SDL_FRect LOGIN_NAME_BOX{ 60.0f, 184.0f, 180.0f, 45.0f };
constexpr SDL_FRect LOGIN_PASS_BOX{ 255.0f, 184.0f, 180.0f, 45.0f };
constexpr SDL_FRect SIGNUP_NAME_BOX{ 150.0f, 143.0f, 195.0f, 33.0f };
constexpr SDL_FRect SIGNUP_PASS_BOX{ 151.0f, 215.0f, 195.0f, 36.0f };

constexpr SDL_FRect LOGIN_SUBMIT_BUTTON{ 253.0f, 250.0f, 185.0f, 48.0f };
constexpr SDL_FRect LOGIN_BACK_BUTTON{ 253.0f, 365.0f, 185.0f, 51.0f };
constexpr SDL_FRect SIGNUP_BACK_BUTTON{ 58.0f, 338.0f, 184.0f, 40.0f };
constexpr SDL_FRect SIGNUP_SUBMIT_BUTTON{ 255.0f, 338.0f, 184.0f, 40.0f };
}  // namespace

LoginSignupScreen::LoginSignupScreen(SDL_Renderer* renderer, SDL_Window* window,
                                     TTF_Font* font, AuthSession& session,
                                     std::string host, std::string port)
    : renderer_(renderer), window_(window), font_(font), session_(session),
      host_(std::move(host)), port_(std::move(port)),
      name_field_(LOGIN_NAME_BOX, /*password=*/false),
      password_field_(LOGIN_PASS_BOX, /*password=*/true) {
    // Cada pantalla fija su propia presentacion logica (contrato del ScreenManager).
    SDL_SetRenderLogicalPresentation(renderer_, LOGIN_W, LOGIN_H,
                                     SDL_LOGICAL_PRESENTATION_LETTERBOX);
    choice_bg_ = load_png(CHOICE_PNG);
    login_bg_ = load_png(LOGIN_PNG);
    signup_bg_ = load_png(SIGNUP_PNG);
}

LoginSignupScreen::~LoginSignupScreen() {
    SDL_StopTextInput(window_);
    if (choice_bg_) { SDL_DestroyTexture(choice_bg_); choice_bg_ = nullptr; }
    if (login_bg_) { SDL_DestroyTexture(login_bg_); login_bg_ = nullptr; }
    if (signup_bg_) { SDL_DestroyTexture(signup_bg_); signup_bg_ = nullptr; }
}

SDL_Texture* LoginSignupScreen::load_png(const char* path) {
    // Tolerante: si la PNG no esta (nombres provisorios), devuelve nullptr y la
    // vista se dibuja con fondo vacio en vez de crashear. Cambiar a lanzar una
    // excepcion una vez que esten las imagenes definitivas.
    const std::string full = paths::asset(path);
    SDL_Surface* surface = IMG_Load(full.c_str());
    if (!surface) {
        SDL_Log("LoginSignupScreen: no se pudo cargar %s: %s", full.c_str(), SDL_GetError());
        return nullptr;
    }
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer_, surface);
    SDL_DestroySurface(surface);
    if (!texture) {
        SDL_Log("LoginSignupScreen: CreateTextureFromSurface: %s", SDL_GetError());
        return nullptr;
    }
    return texture;
}

void LoginSignupScreen::focus(TextField& field) {
    const bool name_gets_focus = (&field == &name_field_);
    name_field_.set_focus(name_gets_focus);
    password_field_.set_focus(!name_gets_focus);
}

void LoginSignupScreen::enter_form() {
    // El text-input es estado global de la ventana: lo prende la pantalla, una vez.
    apply_form_layout();
    SDL_StartTextInput(window_);
    focus(name_field_);
}

void LoginSignupScreen::apply_form_layout() {
    if (view_ == View::LOGIN_FORM) {
        name_field_.set_box(LOGIN_NAME_BOX);
        password_field_.set_box(LOGIN_PASS_BOX);
    } else if (view_ == View::SIGNUP_FORM) {
        name_field_.set_box(SIGNUP_NAME_BOX);
        password_field_.set_box(SIGNUP_PASS_BOX);
    }
}

void LoginSignupScreen::leave_form() {
    name_field_.set_focus(false);
    password_field_.set_focus(false);
    SDL_StopTextInput(window_);
    view_ = View::CHOICE;
}

void LoginSignupScreen::handleEvent(const SDL_Event& event) {
    if (event.type == SDL_EVENT_QUIT) {
        next_ = ScreenState::EXIT;
        return;
    }

    if (view_ == View::CHOICE) {
        handle_choice_event(event);
    } else {
        handle_form_event(event);
    }
}

void LoginSignupScreen::handle_choice_event(const SDL_Event& event) {
    if (event.type == SDL_EVENT_KEY_DOWN &&
        event.key.scancode == SDL_SCANCODE_ESCAPE) {
        next_ = ScreenState::EXIT;
        return;
    }

    if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN &&
        event.button.button == SDL_BUTTON_LEFT) {
        float x = 0.0f;
        float y = 0.0f;
        SDL_RenderCoordinatesFromWindow(renderer_, event.button.x, event.button.y, &x, &y);

        if (contains(CHOICE_SIGNUP_BUTTON, x, y)) {
            view_ = View::SIGNUP_FORM;
            error_message_.clear();
            enter_form();
        } else if (contains(CHOICE_LOGIN_BUTTON, x, y)) {
            view_ = View::LOGIN_FORM;
            error_message_.clear();
            enter_form();
        }
    }
}

void LoginSignupScreen::handle_form_event(const SDL_Event& event) {
    if (event.type == SDL_EVENT_KEY_DOWN) {
        // ESC vuelve a la pantalla de eleccion.
        if (event.key.scancode == SDL_SCANCODE_ESCAPE) {
            leave_form();
            error_message_.clear();
            return;
        }
        // TAB alterna el foco entre nombre y contrasena.
        if (event.key.scancode == SDL_SCANCODE_TAB) {
            focus(name_field_.is_focused() ? password_field_ : name_field_);
            return;
        }
        // ENTER confirma segun la vista.
        if (event.key.scancode == SDL_SCANCODE_RETURN ||
            event.key.scancode == SDL_SCANCODE_KP_ENTER) {
            submit_current_form();
            return;
        }
    }

    if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN &&
        event.button.button == SDL_BUTTON_LEFT) {
        float x = 0.0f;
        float y = 0.0f;
        SDL_RenderCoordinatesFromWindow(renderer_, event.button.x, event.button.y, &x, &y);
        if (view_ == View::LOGIN_FORM && contains(LOGIN_SUBMIT_BUTTON, x, y)) {
            submit_current_form();
        } else if (view_ == View::LOGIN_FORM && contains(LOGIN_BACK_BUTTON, x, y)) {
            leave_form();
            error_message_.clear();
        } else if (view_ == View::SIGNUP_FORM && contains(SIGNUP_SUBMIT_BUTTON, x, y)) {
            submit_current_form();
        } else if (view_ == View::SIGNUP_FORM && contains(SIGNUP_BACK_BUTTON, x, y)) {
            leave_form();
            error_message_.clear();
        } else if (name_field_.hit(x, y)) {
            focus(name_field_);
        } else if (password_field_.hit(x, y)) {
            focus(password_field_);
        }
        return;
    }

    // El resto de los eventos (texto, backspace) va al campo enfocado.
    name_field_.handle_event(event);
    password_field_.handle_event(event);
}

bool LoginSignupScreen::ensure_connected() {
    if (session_.protocol) {
        return true;
    }
    try {
        Socket skt(host_.c_str(), port_.c_str());
        session_.protocol = std::make_unique<ClientProtocol>(std::move(skt));
        return true;
    } catch (const std::exception&) {
        error_message_ = "No se pudo conectar al servidor";
        return false;
    }
}

void LoginSignupScreen::try_login() {
    if (name_field_.value().empty() || password_field_.value().empty()) {
        error_message_ = "Ingresa nombre y contrasena";
        return;
    }
    if (!ensure_connected()) {
        return;
    }

    ClientCmd cmd;
    cmd.set_message_type(MSG_LOGIN);
    cmd.set_player_name(name_field_.value());
    cmd.set_password(password_field_.value());

    try {
        session_.protocol->send_command(cmd);

        // Respuesta sincrona: exactamente MSG_AUTH_ERROR o MSG_CONFIRM_SESSION.
        GameMsg msg(MSG_AUTH_ERROR);
        if (!session_.protocol->receive_event(msg)) {
            error_message_ = "Se perdio la conexion";
            return;
        }

        if (msg.get_type() == MSG_AUTH_ERROR) {
            error_message_ = msg.get_chat_content();
            return;
        }
        if (msg.get_type() == MSG_CONFIRM_SESSION) {
            session_.name = msg.get_player_name();
            session_.race = msg.get_race();
            session_.klass = msg.get_class();
            session_.clan = msg.get_chat_content();
            session_.authenticated = true;
            error_message_.clear();
            next_ = ScreenState::GAME;
            return;
        }
        // Cualquier otro tipo es inesperado en este punto del handshake.
        error_message_ = "Respuesta inesperada del servidor";
    } catch (const std::exception& e) {
        error_message_ = std::string("Error de red: ") + e.what();
    }
}

void LoginSignupScreen::submit_current_form() {
    if (view_ == View::LOGIN_FORM) {
        try_login();
    } else if (view_ == View::SIGNUP_FORM) {
        submit_signup();
    }
}

void LoginSignupScreen::submit_signup() {
    if (name_field_.value().empty() || password_field_.value().empty()) {
        error_message_ = "Ingresa nombre y contrasena";
        return;
    }

    session_.name = name_field_.value();
    session_.password = password_field_.value();
    error_message_.clear();
    next_ = ScreenState::CHARACTER_CREATION;
}

void LoginSignupScreen::update() {}

void LoginSignupScreen::render() {
    SDL_SetRenderDrawColor(renderer_, 0, 0, 0, 255);
    SDL_RenderClear(renderer_);

    // Fondo segun la vista activa (la PNG ocupa toda la pantalla logica).
    SDL_Texture* bg = nullptr;
    switch (view_) {
        case View::CHOICE:      bg = choice_bg_; break;
        case View::LOGIN_FORM:  bg = login_bg_;  break;
        case View::SIGNUP_FORM: bg = signup_bg_; break;
    }
    if (bg) {
        SDL_RenderTexture(renderer_, bg, nullptr, nullptr);
    }

    if (view_ == View::LOGIN_FORM || view_ == View::SIGNUP_FORM) {
        name_field_.render(renderer_, font_);
        password_field_.render(renderer_, font_);
    }

    if (!error_message_.empty()) {
        draw_text(renderer_, font_, error_message_.c_str(), 96.0f, 420.0f, COLOR_ERROR);
    }
}

ScreenState LoginSignupScreen::nextState() const { return next_; }
