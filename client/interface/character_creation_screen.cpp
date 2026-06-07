#include "character_creation_screen.h"

#include <array>
#include <utility>

#include "../comunication/clientProtocol.h"
#include "../../common/socket/socket.h"
#include "../../common/commands/clientCmd.h"
#include "../../common/commands/gameMsg.h"
#include "../../common/constants/protocol_constants.h"

static constexpr int CREATE_W = 493;
static constexpr int CREATE_H = 479;

// Strings del protocolo (mapeados por el serializer del cliente).
static const std::array<const char*, 4> RACES = {"human", "elf", "dwarf", "gnome"};
static const std::array<const char*, 4> CLASSES = {"wizard", "cleric", "paladin", "warrior"};

CharacterCreationScreen::CharacterCreationScreen(SDL_Renderer* renderer, SDL_Window* window,
                                                 TTF_Font* font, AuthSession& session,
                                                 std::string host, std::string port)
    : renderer(renderer), window(window), font(font), session(session),
      host(std::move(host)), port(std::move(port)) {
    SDL_SetRenderLogicalPresentation(renderer, CREATE_W, CREATE_H,
                                     SDL_LOGICAL_PRESENTATION_LETTERBOX);
}

void CharacterCreationScreen::draw_text(const char* text, float x, float y, SDL_Color color) {
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

bool CharacterCreationScreen::ensure_protocol() {
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

void CharacterCreationScreen::try_register() {
    if (session.name.empty() || session.password.empty()) {
        error_message = "Faltan nombre/contrasena (volve al login)";
        return;
    }
    if (!ensure_protocol()) {
        return;
    }

    const std::string race = RACES[selected_race];
    const std::string klass = CLASSES[selected_class];

    ClientCmd cmd;
    cmd.set_message_type(MSG_REGISTER);
    cmd.set_player_name(session.name);
    cmd.set_password(session.password);
    cmd.set_race(race);
    cmd.set_class(klass);
    try {
        session.protocol->send_command(cmd);

        GameMsg msg(0);
        if (!session.protocol->receive_event(msg)) {
            error_message = "Conexion cerrada por el servidor";
            return;
        }
        if (msg.get_type() == MSG_AUTH_ERROR) {
            error_message = msg.get_chat_content();
            next = ScreenState::LOGIN_SIGNUP;
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

void CharacterCreationScreen::handleEvent(const SDL_Event& event) {
    switch (event.type) {
        case SDL_EVENT_QUIT:
            next = ScreenState::EXIT;
            break;
        case SDL_EVENT_KEY_DOWN:
            switch (event.key.scancode) {
                case SDL_SCANCODE_ESCAPE:
                    next = ScreenState::LOGIN_SIGNUP;
                    break;
                case SDL_SCANCODE_LEFT:
                    selected_race = (selected_race + RACES.size() - 1) % RACES.size();
                    break;
                case SDL_SCANCODE_RIGHT:
                    selected_race = (selected_race + 1) % RACES.size();
                    break;
                case SDL_SCANCODE_UP:
                    selected_class = (selected_class + CLASSES.size() - 1) % CLASSES.size();
                    break;
                case SDL_SCANCODE_DOWN:
                    selected_class = (selected_class + 1) % CLASSES.size();
                    break;
                case SDL_SCANCODE_RETURN:
                    try_register();
                    break;
                default:
                    break;
            }
            break;
        default:
            break;
    }
}

void CharacterCreationScreen::update() {}

void CharacterCreationScreen::render() {
    SDL_SetRenderDrawColor(renderer, 22, 18, 28, 255);
    SDL_RenderClear(renderer);

    const SDL_Color white{235, 235, 235, 255};
    const SDL_Color yellow{255, 220, 80, 255};
    const SDL_Color red{230, 80, 80, 255};

    draw_text("CREACION DE PERSONAJE (stub)", 40.0f, 40.0f, yellow);
    draw_text("Izq/Der: raza  |  Arr/Aba: clase", 40.0f, 70.0f, white);
    draw_text("Enter: Confirmar  |  Esc: volver", 40.0f, 92.0f, white);

    std::string race_line = std::string("Raza:  ") + RACES[selected_race];
    std::string class_line = std::string("Clase: ") + CLASSES[selected_class];
    draw_text(race_line.c_str(), 40.0f, 170.0f, yellow);
    draw_text(class_line.c_str(), 40.0f, 200.0f, yellow);

    if (!error_message.empty()) {
        draw_text(error_message.c_str(), 40.0f, 260.0f, red);
    }
}

ScreenState CharacterCreationScreen::nextState() const {
    return next;
}
