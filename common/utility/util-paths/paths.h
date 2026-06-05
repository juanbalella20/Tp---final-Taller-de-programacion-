#ifndef COMMON_UTILITY_PATHS_H
#define COMMON_UTILITY_PATHS_H

#include <string>

// Resolucion de rutas segun el estandar XDG Base Directory.
//
// El binario instalado no corre desde el repo, asi que no puede usar rutas
// relativas como "imagenes/foo.png". Estas funciones devuelven la ubicacion
// real de los recursos:
//
//   assets -> ~/.local/share/argentum/   (o $XDG_DATA_HOME/argentum/)
//   config -> ~/.config/argentum/        (o $XDG_CONFIG_HOME/argentum/)
//
// Para facilitar el desarrollo sin instalar, si el recurso no existe en la
// ruta XDG se cae a una ruta relativa al directorio de trabajo actual (el
// layout del repo). Asi `client` corrido desde la raiz del repo sigue
// funcionando igual que antes.
namespace paths {

// Nombre del TP (el "NAME" del enunciado). Se usa como subcarpeta dentro de
// ~/.local/share y ~/.config.
constexpr const char* APP_NAME = "argentum";

// Ruta absoluta a un asset instalado. `relative` es la ruta dentro de la
// carpeta de datos, p.ej. "imagenes/3.png" o "data/maps/desert/map.toml".
std::string asset(const std::string& relative);

// Ruta absoluta a un archivo de configuracion del usuario, p.ej.
// "config.toml". `relative` es la ruta dentro de ~/.config/argentum/.
std::string config(const std::string& relative);

}  // namespace paths

#endif  // COMMON_UTILITY_PATHS_H
