#include "paths.h"

#include <cstdlib>
#include <filesystem>

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

    // El PNG esta fuera de la carpeta de recursos: guardamos solo el nombre. El
    // cliente lo buscara en la raiz de recursos. Avisamos por stderr para que el
    // mapeador note que deberia mover/elegir el PNG desde la carpeta correcta.
    return png_c.filename().string();
}

std::string resolve_resource(const std::string& relative) {
    fs::path p(relative);
    if (p.is_absolute())
        return relative;  // mapas viejos con ruta absoluta: sin tocar
    return (fs::path(resources_dir()) / p).string();
}

}  // namespace paths
