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
constexpr const char *APP_NAME = "argentum";

// Ruta absoluta a un asset instalado. `relative` es la ruta dentro de la
// carpeta de datos, p.ej. "imagenes/3.png" o "data/maps/desert/map.toml".
std::string asset(const std::string &relative);

// Ruta absoluta a un archivo de configuracion del usuario, p.ej.
// "config.toml". `relative` es la ruta dentro de ~/.config/argentum/.
std::string config(const std::string &relative);

// --- Recursos compartidos editor <-> cliente --------------------------------
// Nombre de la variable de entorno que apunta a la carpeta de recursos.
constexpr const char *RESOURCES_DIR_ENV = "ARGENTUM_RESOURCES_DIR";

// Ruta absoluta a la carpeta raiz de recursos (ARGENTUM_RESOURCES_DIR o el
// cwd).
std::string resources_dir();

// Subcarpeta "assets/tilesets" donde viven todos los tilesets (PNG) que el
// editor pre-carga y el cliente conoce.
constexpr const char *TILESETS_SUBDIR = "assets/tilesets";
std::string tilesets_dir();

// Convierte una ruta absoluta a un PNG (la que el usuario eligio en el editor)
// en una ruta relativa a resources_dir(), para guardar en el .bin. Si el PNG no
// esta debajo de la carpeta de recursos pero tiene un componente "data" (esta
// dentro de un checkout del repo), guarda la cola "data/..." que es igual en
// cualquier maquina. Como ultimo recurso devuelve solo el nombre del archivo
// (y el cliente lo buscara en la raiz de recursos).
std::string resource_relative(const std::string &absolute_png);

// Resuelve una ruta de tileset guardada en el .bin (relativa a la carpeta de
// recursos) a una ruta absoluta cargable. Si viniera absoluta (mapas viejos):
// se usa tal cual solo si existe en esta maquina; si no, se rescata la cola
// "data/..." y se resuelve contra la carpeta de recursos local, para que un
// .bin guardado en otra compu cargue igual desde este repo.
std::string resolve_resource(const std::string &relative);

} // namespace paths

#endif // COMMON_UTILITY_PATHS_H
