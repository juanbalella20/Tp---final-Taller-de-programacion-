#ifndef SCREEN_MANAGER_H
#define SCREEN_MANAGER_H

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

#include <memory>
#include <string>

#include "Screen.h"
#include "auth_session.h"

// Dueño del SDL_Window / SDL_Renderer / TTF_Font COMPARTIDOS por todas las
// pantallas. Crea esos recursos una sola vez (no parpadea entre pantallas) y
// drivea el ciclo de pantallas pre-juego:
//
//   WELCOME -> LOGIN_SIGNUP -> (CHARACTER_CREATION si "Crear cuenta") -> GAME
//
// run() corre hasta que se alcanza GAME (auth OK; session.authenticated == true)
// o EXIT (el usuario cerro). NO maneja el GAME en si: devuelve el estado final
// y ClientApp arranca los threads de red y construye ClientGUI sobre el mismo
// window/renderer/font (accesibles via los getters).
class ScreenManager {
 private:
    static constexpr char WINDOW_NAME[] = "Argentum Online";
    static constexpr char FONT_PATH[] = "fonts/StackSansText-Medium.ttf";
    static constexpr int INITIAL_WIDTH = 1280;
    static constexpr int INITIAL_HEIGHT = 720;
    static constexpr int FONT_SIZE = 16;

    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;
    TTF_Font* font = nullptr;

    std::string host;
    std::string port;

    // Construye la pantalla correspondiente al estado dado (nullptr para GAME/EXIT).
    std::unique_ptr<Screen> make_screen(ScreenState state, AuthSession& session);

 public:
    ScreenManager(std::string host, std::string port);
    ~ScreenManager();

    ScreenManager(const ScreenManager&) = delete;
    ScreenManager& operator=(const ScreenManager&) = delete;

    // Corre el ciclo de pantallas pre-juego. Devuelve GAME (auth OK) o EXIT.
    ScreenState run(AuthSession& session);

    SDL_Window* get_window() const { return window; }
    SDL_Renderer* get_renderer() const { return renderer; }
    TTF_Font* get_font() const { return font; }
};

#endif  // SCREEN_MANAGER_H
