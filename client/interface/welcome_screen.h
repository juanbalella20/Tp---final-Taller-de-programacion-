#ifndef WELCOME_SCREEN_H
#define WELCOME_SCREEN_H

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

#include "Screen.h"

struct WindowSettings {
    int width = 1280;
    int height = 720;
    bool fullscreen = false;
};

struct LauncherResult {
    bool start_game = false;
    WindowSettings settings;
};

// Pantalla de configuracion/launcher. Implementa Screen: NO crea ni destruye el
// window/renderer (los recibe del ScreenManager); solo carga sus propias
// texturas y su fuente de tamano fijo. Al confirmar transiciona a LOGIN_SIGNUP;
// al cerrar, a EXIT. Los WindowSettings elegidos quedan accesibles via
// get_result() para que el ScreenManager los aplique a la ventana compartida.
class WelcomeScreen : public Screen {
 private:
    static constexpr char WINDOW_NAME[] = "Argentum Online";
    static constexpr char BACKGROUND_PATH[] =
        "imagenes/login/en_ventanalauncher.bmp";
    static constexpr char CONFIG_PATH[] =
        "imagenes/login/config_pantalla2.png";
    static constexpr char FONT_PATH[] =
        "fonts/StackSansText-Medium.ttf";
    static constexpr int LAUNCHER_WIDTH = 493;
    static constexpr int LAUNCHER_HEIGHT = 479;
    static constexpr int CONFIG_IMAGE_WIDTH = 1509;
    static constexpr int CONFIG_IMAGE_HEIGHT = 1173;
    static constexpr int RESOLUTION_OPTION_COUNT = 4;
    static constexpr float RESOLUTION_OPTION_HEIGHT = 82.0f;
    static constexpr SDL_FRect CONFIG_POPUP{
        20.0f, 64.0f, 453.0f, 352.0f};
    static constexpr SDL_FRect START_BUTTON{
        163.0f, 356.0f, 166.0f, 43.0f};
    static constexpr SDL_FRect CLOSE_BUTTON{
        464.0f, 0.0f, 29.0f, 29.0f};
    static constexpr SDL_FRect SETTINGS_BUTTON{
        333.0f, 211.0f, 130.0f, 29.0f};
    static constexpr SDL_FRect CONFIG_CLOSE_BUTTON{
        1438.0f, 8.0f, 64.0f, 60.0f};
    static constexpr SDL_FRect CONFIG_CANCEL_BUTTON{
        199.0f, 855.0f, 535.0f, 110.0f};
    static constexpr SDL_FRect CONFIG_APPLY_BUTTON{
        779.0f, 855.0f, 535.0f, 110.0f};
    static constexpr SDL_FRect RESOLUTION_BUTTON{
        631.0f, 395.0f, 717.0f, 110.0f};
    static constexpr SDL_FRect WINDOWED_BUTTON{
        1021.0f, 612.0f, 86.0f, 82.0f};
    static constexpr SDL_FRect FULLSCREEN_BUTTON{
        629.0f, 612.0f, 82.0f, 82.0f};

    // No-owning: pertenecen al ScreenManager.
    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;

    SDL_Texture* background = nullptr;
    SDL_Texture* config = nullptr;
    TTF_Font* config_font = nullptr;  // propia (tamano fijo); cerrada en el dtor.
    bool media_loaded = false;
    bool showing_config = false;
    bool resolution_dropdown_open = false;
    LauncherResult result;
    WindowSettings pending_settings;
    int selected_resolution_index = 0;
    int pending_resolution_index = 0;
    ScreenState next = ScreenState::WELCOME;

    static bool contains(const SDL_FRect& rect, float x, float y);
    static bool toConfigCoordinates(float x, float y, float& config_x, float& config_y);
    static SDL_FRect toPopupRect(const SDL_FRect& config_rect);
    void loadMedia();
    void freeMedia();
    void handleMainEvent(const SDL_Event& event);
    void handleConfigEvent(const SDL_Event& event);
    void showMainScreen();
    void showConfigScreen();
    void set_fullscreen(bool fs);
    void setResolution(int index);
    void applySettings();
    void discardSettings();
    SDL_FRect resolutionOptionRect(int index) const;
    int resolutionOptionAt(float config_x, float config_y) const;
    const char* resolutionLabel(int index) const;
    void drawConfigControls();
    void drawResolutionDropdown();
    void drawTick(const SDL_FRect& config_rect);
    void drawText(const char* text, float x, float y, SDL_Color color);

 public:
    // win/rend NO-owning. La fuente compartida del manager se ignora aca: esta
    // pantalla usa una fuente propia de tamano fijo por su layout.
    WelcomeScreen(SDL_Renderer* renderer, SDL_Window* window, TTF_Font* font);
    ~WelcomeScreen() override;

    WelcomeScreen(const WelcomeScreen&) = delete;
    WelcomeScreen& operator=(const WelcomeScreen&) = delete;

    void handleEvent(const SDL_Event& event) override;
    void update() override;
    void render() override;
    ScreenState nextState() const override;

    // WindowSettings elegidos por el usuario (para que el manager los aplique).
    const LauncherResult& get_result() const { return result; }
};

#endif
