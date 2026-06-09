#include "character_creation_screen.h"

#include <array>
#include <cstdio>
#include <stdexcept>
#include <utility>
#include <iostream>
#include <SDL3_image/SDL_image.h>

#include "paths.h"
#include "ui_helpers.h"
#include "../comunication/clientProtocol.h"
#include "../../common/socket/socket.h"
#include "../../common/commands/clientCmd.h"
#include "../../common/commands/gameMsg.h"
#include "../../common/constants/game_config.h"
#include "../../common/constants/protocol_constants.h"

namespace {

struct SelectionOption {
    const char* protocol_value;
    const char* label;
};

const std::array<SelectionOption, 4> RACES{{
    {"human", "Humano"},
    {"elf", "Elfo"},
    {"dwarf", "Enano"},
    {"gnome", "Gnomo"}
}};

const std::array<SelectionOption, 4> CLASSES{{
    {"wizard", "Mago"},
    {"cleric", "Clerigo"},
    {"paladin", "Paladin"},
    {"warrior", "Guerrero"}
}};

constexpr SDL_Color COLOR_TEXT{235, 228, 210, 255};
constexpr SDL_Color COLOR_ERROR{230, 80, 70, 255};
constexpr SDL_Color COLOR_DROPDOWN_BORDER{178, 156, 112, 255};
constexpr SDL_Color COLOR_DROPDOWN_FILL{13, 11, 10, 235};
constexpr SDL_Color COLOR_DROPDOWN_SELECTED{92, 36, 26, 235};

constexpr SDL_FRect CLASS_SELECTOR{180.0f, 205.0f, 520.0f, 95.0f};
constexpr SDL_FRect CLASS_FIELD{290.0f, 215.0f, 300.0f, 75.0f};
constexpr SDL_FRect RACE_SELECTOR{750.0f, 205.0f, 525.0f, 95.0f};
constexpr SDL_FRect RACE_FIELD{860.0f, 215.0f, 305.0f, 75.0f};
constexpr SDL_FRect BACK_BUTTON{140.0f, 850.0f, 500.0f, 120.0f};
constexpr SDL_FRect CREATE_BUTTON{780.0f, 850.0f, 520.0f, 120.0f};

constexpr float DROPDOWN_GAP = 10.0f;
constexpr float DROPDOWN_OPTION_H = 58.0f;
constexpr float ERROR_Y = 790.0f;

// Seccion "Atributos" del fondo: centro vertical de cada fila (Fuerza,
// Agilidad, Inteligencia, Constitucion, Carisma) en pixeles del PNG, que
// coincide 1:1 con la resolucion logica CREATE_W x CREATE_H. El valor se
// dibuja a la derecha del adorno que cierra cada fila.
constexpr std::array<float, 5> ATTR_ROW_CY{464.0f, 536.0f, 610.0f, 684.0f, 757.0f};
constexpr float ATTR_VALUE_X = 815.0f;
constexpr float ATTR_VALUE_W = 110.0f;
constexpr float ATTR_VALUE_H = 44.0f;

// Mismo orden que RACES: human, elf, dwarf, gnome.
const RaceConfig& race_config_at(int index) {
    const GameConfig& cfg = GameConfig::instance();
    switch (index) {
        case 0: return cfg.human;
        case 1: return cfg.elf;
        case 2: return cfg.dwarf;
        default: return cfg.gnome;
    }
}

// Mismo orden que CLASSES: wizard, cleric, paladin, warrior.
const ClassConfig& class_config_at(int index) {
    const GameConfig& cfg = GameConfig::instance();
    switch (index) {
        case 0: return cfg.wizard;
        case 1: return cfg.cleric;
        case 2: return cfg.paladin;
        default: return cfg.warrior;
    }
}

std::string format_stat(float value) {
    const int entero = static_cast<int>(value);
    if (value == static_cast<float>(entero)) {
        return std::to_string(entero);
    }
    char buf[16];
    std::snprintf(buf, sizeof(buf), "%.1f", value);
    return std::string(buf);
}

SDL_FRect dropdown_rect(const SDL_FRect& field, int option_count) {
    return SDL_FRect{
        field.x,
        field.y + field.h + DROPDOWN_GAP,
        field.w,
        DROPDOWN_OPTION_H * option_count
    };
}

void draw_centered_text(SDL_Renderer* renderer, TTF_Font* font, const char* text,
                        const SDL_FRect& box, SDL_Color color) {
    if (!renderer || !font || !text || text[0] == '\0') {
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

    const float w = static_cast<float>(surface->w);
    const float h = static_cast<float>(surface->h);
    SDL_FRect dst{
        box.x + (box.w - w) * 0.5f,
        box.y + (box.h - h) * 0.5f,
        w,
        h
    };
    SDL_DestroySurface(surface);
    SDL_RenderTexture(renderer, texture, nullptr, &dst);
    SDL_DestroyTexture(texture);
}

void draw_dropdown(SDL_Renderer* renderer, TTF_Font* font, const SDL_FRect& field,
                   const std::array<SelectionOption, 4>& options, int selected) {
    const SDL_FRect panel = dropdown_rect(field, static_cast<int>(options.size()));

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, COLOR_DROPDOWN_FILL.r, COLOR_DROPDOWN_FILL.g,
                           COLOR_DROPDOWN_FILL.b, COLOR_DROPDOWN_FILL.a);
    SDL_RenderFillRect(renderer, &panel);
    SDL_SetRenderDrawColor(renderer, COLOR_DROPDOWN_BORDER.r, COLOR_DROPDOWN_BORDER.g,
                           COLOR_DROPDOWN_BORDER.b, COLOR_DROPDOWN_BORDER.a);
    SDL_RenderRect(renderer, &panel);

    for (int i = 0; i < static_cast<int>(options.size()); ++i) {
        SDL_FRect option{
            panel.x,
            panel.y + DROPDOWN_OPTION_H * i,
            panel.w,
            DROPDOWN_OPTION_H
        };
        if (i == selected) {
            SDL_SetRenderDrawColor(renderer, COLOR_DROPDOWN_SELECTED.r,
                                   COLOR_DROPDOWN_SELECTED.g,
                                   COLOR_DROPDOWN_SELECTED.b,
                                   COLOR_DROPDOWN_SELECTED.a);
            SDL_RenderFillRect(renderer, &option);
        }

        SDL_SetRenderDrawColor(renderer, COLOR_DROPDOWN_BORDER.r,
                               COLOR_DROPDOWN_BORDER.g,
                               COLOR_DROPDOWN_BORDER.b, 120);
        SDL_RenderRect(renderer, &option);
        draw_centered_text(renderer, font, options[i].label, option, COLOR_TEXT);
    }
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
}

}  // namespace

CharacterCreationScreen::CharacterCreationScreen(SDL_Renderer* renderer, SDL_Window* window,
                                                 TTF_Font* font, AuthSession& session,
                                                 std::string host, std::string port)
    : renderer(renderer), window(window), font(font), session(session),
      host(std::move(host)), port(std::move(port)) {
    SDL_SetRenderLogicalPresentation(renderer, CREATE_W, CREATE_H,
                                     SDL_LOGICAL_PRESENTATION_LETTERBOX);

    background = load_png(BACKGROUND_PNG);

    const std::string font_path = paths::asset(FONT_PATH);
    label_font = TTF_OpenFont(font_path.c_str(), LABEL_FONT_SIZE);
    if (!label_font) {
        SDL_Log("CharacterCreationScreen: no se pudo cargar %s: %s",
                font_path.c_str(), SDL_GetError());
    }
    message_font = TTF_OpenFont(font_path.c_str(), MESSAGE_FONT_SIZE);
    if (!message_font) {
        SDL_Log("CharacterCreationScreen: no se pudo cargar %s: %s",
                font_path.c_str(), SDL_GetError());
    }
    stats_font = TTF_OpenFont(font_path.c_str(), STATS_FONT_SIZE);
    if (!stats_font) {
        SDL_Log("CharacterCreationScreen: no se pudo cargar %s: %s",
                font_path.c_str(), SDL_GetError());
    }

    update_attribute_values();
}

CharacterCreationScreen::~CharacterCreationScreen() {
    if (background) {
        SDL_DestroyTexture(background);
        background = nullptr;
    }
    if (label_font) {
        TTF_CloseFont(label_font);
        label_font = nullptr;
    }
    if (message_font) {
        TTF_CloseFont(message_font);
        message_font = nullptr;
    }
    if (stats_font) {
        TTF_CloseFont(stats_font);
        stats_font = nullptr;
    }
}

SDL_Texture* CharacterCreationScreen::load_png(const char* path) {
    const std::string full = paths::asset(path);
    SDL_Surface* surface = IMG_Load(full.c_str());
    if (!surface) {
        throw std::runtime_error("No se pudo cargar " + full + ": " + SDL_GetError());
    }
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_DestroySurface(surface);
    if (!texture) {
        throw std::runtime_error(std::string("SDL_CreateTextureFromSurface: ") +
                                 SDL_GetError());
    }
    return texture;
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
        error_message = "Faltan nombre/contraseña (volve al login)";
        return;
    }
    if (!ensure_protocol()) {
        return;
    }

    const std::string race = RACES[selected_race].protocol_value;
    const std::string klass = CLASSES[selected_class].protocol_value;

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

int CharacterCreationScreen::class_option_at(float x, float y) const {
    const SDL_FRect panel = dropdown_rect(CLASS_FIELD, static_cast<int>(CLASSES.size()));
    if (!contains(panel, x, y)) {
        return -1;
    }
    const int option = static_cast<int>((y - panel.y) / DROPDOWN_OPTION_H);
    return option < static_cast<int>(CLASSES.size()) ? option : -1;
}

int CharacterCreationScreen::race_option_at(float x, float y) const {
    const SDL_FRect panel = dropdown_rect(RACE_FIELD, static_cast<int>(RACES.size()));
    if (!contains(panel, x, y)) {
        return -1;
    }
    const int option = static_cast<int>((y - panel.y) / DROPDOWN_OPTION_H);
    return option < static_cast<int>(RACES.size()) ? option : -1;
}

void CharacterCreationScreen::handle_mouse_click(float x, float y) {
    if (class_dropdown_open) {
        const int option = class_option_at(x, y);
        if (option >= 0) {
            selected_class = option;
            std::cout << "[character]: " << CLASSES[selected_class].protocol_value << std::endl;
            update_attribute_values();
            error_message.clear();
        }
        class_dropdown_open = false;
        return;
    }

    if (race_dropdown_open) {
        const int option = race_option_at(x, y);
        if (option >= 0) {
            selected_race = option;
            update_attribute_values();
            error_message.clear();
        }
        race_dropdown_open = false;
        return;
    }

    if (contains(CLASS_SELECTOR, x, y)) {
        class_dropdown_open = true;
        race_dropdown_open = false;
    } else if (contains(RACE_SELECTOR, x, y)) {
        race_dropdown_open = true;
        class_dropdown_open = false;
    } else if (contains(BACK_BUTTON, x, y)) {
        next = ScreenState::LOGIN_SIGNUP;
    } else if (contains(CREATE_BUTTON, x, y)) {
        try_register();
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
                    if (class_dropdown_open || race_dropdown_open) {
                        class_dropdown_open = false;
                        race_dropdown_open = false;
                    } else {
                        next = ScreenState::LOGIN_SIGNUP;
                    }
                    break;
                case SDL_SCANCODE_RETURN:
                case SDL_SCANCODE_KP_ENTER:
                    try_register();
                    break;
                default:
                    break;
            }
            break;
        case SDL_EVENT_MOUSE_BUTTON_DOWN:
            if (event.button.button == SDL_BUTTON_LEFT) {
                float x = 0.0f;
                float y = 0.0f;
                SDL_RenderCoordinatesFromWindow(
                    renderer, event.button.x, event.button.y, &x, &y);
                handle_mouse_click(x, y);
            }
            break;
        default:
            break;
    }
}

void CharacterCreationScreen::update() {}

void CharacterCreationScreen::render() {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    if (background) {
        SDL_RenderTexture(renderer, background, nullptr, nullptr);
    }

    draw_selection_values();
    draw_attribute_values();
    draw_dropdowns();
    draw_error_message();
}

void CharacterCreationScreen::draw_selection_values() {
    TTF_Font* value_font = label_font ? label_font : font;
    draw_centered_text(renderer, value_font, CLASSES[selected_class].label,
                       CLASS_FIELD, COLOR_TEXT);
    draw_centered_text(renderer, value_font, RACES[selected_race].label,
                       RACE_FIELD, COLOR_TEXT);
}

void CharacterCreationScreen::update_attribute_values() {
    // Atributos resultantes de la combinacion raza + clase, sumados igual que
    // los calcula el servidor (ver Player). Carisma no existe en config.toml.
    const RaceConfig& race = race_config_at(selected_race);
    const ClassConfig& klass = class_config_at(selected_class);
    attribute_values[0] = format_stat(race.strength + klass.strength);
    attribute_values[1] = format_stat(race.agility + klass.agility);
    attribute_values[2] = format_stat(race.inteligence + klass.inteligence);
    attribute_values[3] = format_stat(race.endurance + klass.endurance);
    attribute_values[4] = "-";
}

void CharacterCreationScreen::draw_attribute_values() {
    TTF_Font* value_font = stats_font ? stats_font : font;
    for (size_t i = 0; i < attribute_values.size(); ++i) {
        const SDL_FRect box{
            ATTR_VALUE_X,
            ATTR_ROW_CY[i] - ATTR_VALUE_H * 0.5f,
            ATTR_VALUE_W,
            ATTR_VALUE_H
        };
        draw_centered_text(renderer, value_font, attribute_values[i].c_str(),
                           box, COLOR_TEXT);
    }
}

void CharacterCreationScreen::draw_dropdowns() {
    TTF_Font* value_font = label_font ? label_font : font;
    if (class_dropdown_open) {
        draw_dropdown(renderer, value_font, CLASS_FIELD, CLASSES, selected_class);
    } else if (race_dropdown_open) {
        draw_dropdown(renderer, value_font, RACE_FIELD, RACES, selected_race);
    }
}

void CharacterCreationScreen::draw_error_message() {
    if (error_message.empty()) {
        return;
    }
    TTF_Font* error_font = message_font ? message_font : font;
    const SDL_FRect message_box{0.0f, ERROR_Y, static_cast<float>(CREATE_W), 36.0f};
    draw_centered_text(renderer, error_font, error_message.c_str(),
                       message_box, COLOR_ERROR);
}

ScreenState CharacterCreationScreen::nextState() const {
    return next;
}
