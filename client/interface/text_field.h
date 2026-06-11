#ifndef TEXT_FIELD_H
#define TEXT_FIELD_H

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

#include <string>

// Campo de texto editable reutilizable (nombre, contrasena). Encapsula el patron
// de input de minichat.cpp: SDL_StartTextInput/StopTextInput sobre la ventana,
// SDL_EVENT_TEXT_INPUT para escribir, backspace para borrar, y un cursor
// parpadeante mientras esta enfocado.
//
// La pantalla que lo usa es responsable de garantizar que solo UN TextField este
// enfocado a la vez (SDL mantiene un unico estado de text-input por ventana).
class TextField {
 public:
    // `box` es la posicion/tamano del campo en coordenadas logicas del renderer.
    // Si `password` es true, se dibujan asteriscos pero `value()` guarda el real.
    TextField(const SDL_FRect& box, bool password = false);

    // Procesa un evento. Solo actua si el campo esta enfocado:
    //  - SDL_EVENT_TEXT_INPUT  -> agrega el texto tipeado
    //  - BACKSPACE             -> borra el ultimo caracter
    void handle_event(const SDL_Event& event);

    // Enfoca/desenfoca el campo (solo cambia el flag interno). NO toca el
    // text-input de SDL: ese estado es global por ventana, asi que lo maneja la
    // pantalla (un SDL_StartTextInput mientras haya algun campo activo), no cada
    // campo por su cuenta.
    void set_focus(bool focus);
    bool is_focused() const;

    // Reubica el campo manteniendo el texto ya ingresado.
    void set_box(const SDL_FRect& box);

    // Dibuja el marco, el contenido (o asteriscos) y el cursor parpadeante.
    void render(SDL_Renderer* renderer, TTF_Font* font) const;

    const std::string& value() const;
    void clear();

    // ¿El click en (x, y) cae sobre la caja del campo? (para enfoque por mouse)
    bool hit(float x, float y) const;

    const SDL_FRect& box() const;

 private:
    SDL_FRect box_;
    bool password_;
    bool focused_ = false;
    std::string value_;
};

#endif  // TEXT_FIELD_H
