#ifndef CHARACTER_CREATION_SCREEN_H
#define CHARACTER_CREATION_SCREEN_H

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

#include <array>
#include <string>

#include "Screen.h"
#include "auth_session.h"

class CharacterCreationScreen : public Screen {
 private:
    static constexpr char BACKGROUND_PNG[] = "imagenes/login/ch_creation.png";
    static constexpr char FONT_PATH[] = "fonts/StackSansText-Medium.ttf";
    static constexpr int CREATE_W = 1448;
    static constexpr int CREATE_H = 1086;
    static constexpr int LABEL_FONT_SIZE = 44;
    static constexpr int MESSAGE_FONT_SIZE = 28;
    static constexpr int STATS_FONT_SIZE = 38;
    // Filas de la seccion "Atributos" del fondo:
    // Fuerza, Agilidad, Inteligencia, Constitucion, Carisma.
    static constexpr int ATTRIBUTE_COUNT = 5;

    SDL_Renderer* renderer;   // no-owning
    SDL_Window* window;       // no-owning
    TTF_Font* font;           // no-owning
    AuthSession& session;
    std::string host;
    std::string port;

    SDL_Texture* background = nullptr;
    TTF_Font* label_font = nullptr;
    TTF_Font* message_font = nullptr;
    TTF_Font* stats_font = nullptr;

    int selected_race = 0;
    int selected_class = 0;
    // Valores ya formateados para dibujar junto a cada fila de "Atributos".
    std::array<std::string, ATTRIBUTE_COUNT> attribute_values;
    std::string error_message;
    ScreenState next = ScreenState::CHARACTER_CREATION;
    bool class_dropdown_open = false;
    bool race_dropdown_open = false;

    SDL_Texture* load_png(const char* path);
    bool ensure_protocol();
    void try_register();
    void handle_mouse_click(float x, float y);
    int class_option_at(float x, float y) const;
    int race_option_at(float x, float y) const;
    void update_attribute_values();
    void draw_selection_values();
    void draw_attribute_values();
    void draw_dropdowns();
    void draw_error_message();

 public:
    CharacterCreationScreen(SDL_Renderer* renderer, SDL_Window* window, TTF_Font* font,
                            AuthSession& session, std::string host, std::string port);
    ~CharacterCreationScreen() override;

    void handleEvent(const SDL_Event& event) override;
    void update() override;
    void render() override;
    ScreenState nextState() const override;
};

#endif  // CHARACTER_CREATION_SCREEN_H
