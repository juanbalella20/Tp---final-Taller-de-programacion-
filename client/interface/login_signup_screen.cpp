#include "login_signup_screen.h"

#include <utility>

#include "../comunication/clientProtocol.h"
#include "../../common/socket/socket.h"
#include "../../common/commands/clientCmd.h"
#include "../../common/commands/gameMsg.h"
#include "../../common/constants/protocol_constants.h"

// Presentacion logica del login (coincide con el launcher para no saltar).
static constexpr int LOGIN_W = 493;
static constexpr int LOGIN_H = 479;

LoginSignupScreen::LoginSignupScreen(SDL_Renderer* renderer, SDL_Window* window,
                                     TTF_Font* font, AuthSession& session,
                                     std::string host, std::string port)
    : renderer(renderer), window(window), font(font), session(session),
      host(std::move(host)), port(std::move(port)) {
    SDL_SetRenderLogicalPresentation(renderer, LOGIN_W, LOGIN_H,
                                     SDL_LOGICAL_PRESENTATION_LETTERBOX);
    SDL_StartTextInput(window);
}

LoginSignupScreen::~LoginSignupScreen() {
    SDL_StopTextInput(window);
}

void LoginSignupScreen::draw_text(const char* text, float x, float y, SDL_Color color) {
    if (!font || !text || text[0] == '\0') {
        return;
    }
    SDL_Surface* surface = TTF_RenderText_Blended(font, text, 0, color);
    if (!surface) {
        return;
    }
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (!texture) {
        SDL_DestroySurface(surface);
        return;
    }
    SDL_FRect dst{x, y, static_cast<float>(surface->w), static_cast<float>(surface->h)};
    SDL_DestroySurface(surface);
    SDL_RenderTexture(renderer, texture, nullptr, &dst);
    SDL_DestroyTexture(texture);
}

bool LoginSignupScreen::ensure_protocol() {
    if (session.protocol) {
        return true;
    }
    try {
        Socket skt(host.c_str(), port.c_str());
        session.protocol = std::make_unique<ClientProtocol>(std::move(skt));
        return true;
    } catch (const std::exception&) {
        error_message = "No se pudo conectar al servidor";
        return false;
    }
}

void LoginSignupScreen::try_login() {
    if (name.empty() || password.empty()) {
        error_message = "Completa nombre y contrasena";
        return;
    }
    if (!ensure_protocol()) {
        return;
    }

    ClientCmd cmd;
    cmd.set_message_type(MSG_LOGIN);
    cmd.set_player_name(name);
    cmd.set_password(password);
    try {
        session.protocol->send_command(cmd);

        GameMsg msg(0);
        if (!session.protocol->receive_event(msg)) {
            error_message = "Conexion cerrada por el servidor";
            return;
        }
        if (msg.get_type() == MSG_AUTH_ERROR) {
            error_message = msg.get_chat_content();
            return;
        }
        if (msg.get_type() == MSG_CONFIRM_SESSION) {
            session.name = msg.get_player_name();
            session.race = msg.get_race();
            session.klass = msg.get_class();
            session.authenticated = true;
            next = ScreenState::GAME;
        }
    } catch (const std::exception& e) {
        error_message = std::string("Error de red: ") + e.what();
    }
}

void LoginSignupScreen::handleEvent(const SDL_Event& event) {
    switch (event.type) {
        case SDL_EVENT_QUIT:
            next = ScreenState::EXIT;
            break;
        case SDL_EVENT_TEXT_INPUT:
            if (editing_password) {
                password += event.text.text;
            } else {
                name += event.text.text;
            }
            break;
        case SDL_EVENT_KEY_DOWN:
            switch (event.key.scancode) {
                case SDL_SCANCODE_ESCAPE:
                    next = ScreenState::EXIT;
                    break;
                case SDL_SCANCODE_TAB:
                    editing_password = !editing_password;
                    break;
                case SDL_SCANCODE_BACKSPACE: {
                    std::string& field = editing_password ? password : name;
                    if (!field.empty()) {
                        field.pop_back();
                    }
                    break;
                }
                case SDL_SCANCODE_L:
                    // Atajo del stub: intentar LOGIN.
                    break;
                default:
                    break;
            }
            // Enter -> LOGIN; C -> ir a creacion de personaje (registro).
            if (event.key.scancode == SDL_SCANCODE_RETURN) {
                try_login();
            } else if (event.key.scancode == SDL_SCANCODE_F1) {
                session.name = name;
                session.password = password;
                next = ScreenState::CHARACTER_CREATION;
            }
            break;
        default:
            break;
    }
}

void LoginSignupScreen::update() {}

void LoginSignupScreen::render() {
    SDL_SetRenderDrawColor(renderer, 18, 18, 28, 255);
    SDL_RenderClear(renderer);

    const SDL_Color white{235, 235, 235, 255};
    const SDL_Color yellow{255, 220, 80, 255};
    const SDL_Color red{230, 80, 80, 255};

    draw_text("LOGIN / REGISTRO (stub)", 40.0f, 40.0f, yellow);
    draw_text("Tab: cambiar campo  |  Enter: Ingresar", 40.0f, 70.0f, white);
    draw_text("F1: Crear cuenta  |  Esc: salir", 40.0f, 92.0f, white);

    std::string name_line = std::string(editing_password ? "  " : "> ") +
                            "Nombre: " + name;
    std::string pass_line = std::string(editing_password ? "> " : "  ") +
                            "Contrasena: " + std::string(password.size(), '*');
    draw_text(name_line.c_str(), 40.0f, 160.0f, white);
    draw_text(pass_line.c_str(), 40.0f, 190.0f, white);

    if (!error_message.empty()) {
        draw_text(error_message.c_str(), 40.0f, 250.0f, red);
    }
}

ScreenState LoginSignupScreen::nextState() const {
    return next;
}
