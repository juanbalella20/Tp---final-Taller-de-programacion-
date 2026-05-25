#ifndef MINICHAT_H_
#define MINICHAT_H_

#include <string>
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <deque>
#include "../common/queue.h"

#define MAX_LINES 8

struct chatMessage {
    std::string text;
    SDL_Texture* texture;
    float width;
    float height;
};

class MiniChat {

private:
    bool active;
    std::string player_input;
    std::string outbound_msg;
    std::deque<chatMessage> msg_history;

    SDL_Renderer* renderer;
    TTF_Font* font;

    /*
    Si el chat supera MAX_LINES, elimina el mensaje más viejo
    y destruye su textura de la memoria
    */
    void pop_oldest_message();

public:
    MiniChat(SDL_Renderer* renderer, TTF_Font* font);

    bool is_active() const;

    /*
    Si el chat está inactivo, no hace nada (false)
    Si está activo:
    Si es una letra, la suma a lo que se está escribiendo (player_input)
    Si es Backspace, borra la última letra
    Si es Enter, guarda el mensaje terminado en una variable temporal de salida (outbound_msg)
    */
    bool handle_event(const SDL_Event& event);

    /*
    Si está activo, lo desactiva; y viceversa
    Si está activo, llama a SDL_StartTextInput(window)
    Si está inactivo, llama a SDL_StopTextInput(window)
    */
    void toggle_active();

    /*
    Toma el string msg y lo dibuja en una imagen estática que la
    placa de video pueda entender (SDL_Texture)
    */
    SDL_Texture* create_texture_from_msg(const std::string& msg, float& width, float& height);

    /*
    Devuelve true si el jugador escribió algo nuevo, false en caso contrario
    */
    bool has_pending_outbound_message() const;

    /*
    Devuelve el mensaje del jugador
    */
    std::string pop_outbound_message();

    /*
    Saca uno por uno los mensajes que llegaron (inbox)
    Los convierte en imágenes para guardarlos en el historial visual (msg_history)
    */
    void update(Queue<std::string>& inbox);

    /*
    Dibuja y pega las texturas del historial en la pantalla
    */
    void render(int game_width, int game_height);

    ~MiniChat();
};

#endif
