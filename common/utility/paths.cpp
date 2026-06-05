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

}  // namespace paths
