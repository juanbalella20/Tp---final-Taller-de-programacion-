#include "paths.h"

#include <cstdlib>
#include <filesystem>
#include <vector>

namespace {

namespace fs = std::filesystem;

// Devuelve $HOME, o "." si por algun motivo no esta seteada.
fs::path home_dir() {
    if (const char* home = std::getenv("HOME"))
        return fs::path(home);
    return fs::path(".");
}

// Base XDG: usa la variable de entorno si esta seteada (y es absoluta),
// si no el default del estandar relativo a $HOME.
fs::path xdg_base(const char* env_var, const fs::path& default_rel) {
    if (const char* v = std::getenv(env_var)) {
        fs::path p(v);
        if (!p.empty() && p.is_absolute())
            return p;
    }
    return home_dir() / default_rel;
}

// Resuelve `relative` contra la base instalada (XDG). Si ahi no existe,
// cae a la ruta relativa al cwd (layout del repo) para correr sin instalar.
std::string resolve(const fs::path& installed, const std::string& relative) {
    fs::path candidate = installed / relative;
    if (fs::exists(candidate))
        return candidate.string();
    return relative;  // fallback: layout del repo (desarrollo)
}

// Cola de `p` desde su ULTIMO componente "data", p.ej.
// "/home/juan/repo/data/maps/city/5125.png" -> "data/maps/city/5125.png".
// Devuelve "" si el path no tiene componente "data". Sirve para rescatar la
// parte compartida del layout del repo de una ruta absoluta de otra maquina.
std::string data_tail(const fs::path& p) {
    std::vector<fs::path> parts(p.begin(), p.end());
    int last_data = -1;
    for (int i = 0; i < static_cast<int>(parts.size()); ++i) {
        if (parts[i] == "data")
            last_data = i;
    }
    if (last_data < 0)
        return "";
    fs::path tail;
    for (int i = last_data; i < static_cast<int>(parts.size()); ++i)
        tail /= parts[i];
    return tail.generic_string();
}

}  // namespace

namespace paths {

std::string asset(const std::string& relative) {
    fs::path data = xdg_base("XDG_DATA_HOME", fs::path(".local") / "share") /
                    APP_NAME;
    return resolve(data, relative);
}

std::string config(const std::string& relative) {
    fs::path cfg = xdg_base("XDG_CONFIG_HOME", fs::path(".config")) / APP_NAME;
    return resolve(cfg, relative);
}

std::string resources_dir() {
    if (const char* v = std::getenv(RESOURCES_DIR_ENV)) {
        fs::path p(v);
        if (!p.empty() && p.is_absolute())
            return p.string();
    }
    // Sin la env var: layout del repo (cwd), igual que el fallback de asset().
    return fs::current_path().string();
}

std::string resource_relative(const std::string& absolute_png) {
    fs::path png(absolute_png);
    fs::path root(resources_dir());

    // weakly_canonical normaliza ".."/"." y symlinks sin exigir que exista todo
    // el path; cae al lexico si algo falla.
    std::error_code ec;
    fs::path png_c = fs::weakly_canonical(png, ec);
    if (ec) png_c = png.lexically_normal();
    fs::path root_c = fs::weakly_canonical(root, ec);
    if (ec) root_c = root.lexically_normal();

    // Si el PNG cuelga de la carpeta de recursos, devolvemos la ruta relativa.
    fs::path rel = png_c.lexically_relative(root_c);
    if (!rel.empty() && rel.native()[0] != '.') {
        // generic_string -> separadores '/' en todas las plataformas (el .bin
        // es portable; los joins con fs::path aceptan '/' igual).
        return rel.generic_string();
    }

    // Fuera de la carpeta de recursos pero con componente "data": guardamos la
    // cola "data/..." (el layout del repo), que existe igual en el checkout de
    // cualquier maquina. Cubre correr el editor desde un cwd que no es la raiz
    // del repo eligiendo un PNG que SI esta dentro de un repo.
    const std::string tail = data_tail(png_c);
    if (!tail.empty())
        return tail;

    // El PNG esta fuera de la carpeta de recursos: guardamos solo el nombre. El
    // cliente lo buscara en la raiz de recursos. Avisamos por stderr para que el
    // mapeador note que deberia mover/elegir el PNG desde la carpeta correcta.
    return png_c.filename().string();
}

std::string resolve_resource(const std::string& relative) {
    fs::path p(relative);
    fs::path root(resources_dir());
    if (!p.is_absolute())
        return (root / p).string();

    // Ruta absoluta: .bin viejo guardado sin relativizar. Si existe aca (esta
    // es la maquina que lo autoreo), se usa tal cual. Si no, se rescata la
    // cola "data/..." y se resuelve contra la carpeta de recursos LOCAL, asi
    // un .bin con rutas de otra compu carga igual desde este repo.
    std::error_code ec;
    if (fs::exists(p, ec))
        return relative;
    const fs::path norm = p.lexically_normal();
    const std::string tail = data_tail(norm);
    if (!tail.empty())
        return (root / tail).string();
    // Sin componente "data": ultimo intento por nombre de archivo en la raiz.
    const fs::path by_name = root / norm.filename();
    if (fs::exists(by_name, ec))
        return by_name.string();
    return relative;  // irrecuperable: se devuelve igual para un error claro
}

}  // namespace paths
