#include "clan_persistence.h"

#include <filesystem>
#include <fstream>
#include <stdexcept>

#include "../../common/constants/game_constants.h"
#include "paths.h"

ClanPersistence::ClanPersistence(const std::string& data_dir) {
    clans_path = data_dir + PERSIST_CLANS_FILE;
    ensure_file(data_dir);
}

void ClanPersistence::ensure_file(const std::string& data_dir) const {
    // Crea el directorio si no existe (modo 0755; ignora EEXIST).
    std::filesystem::create_directories(paths::config(data_dir));
    // "Toca" el archivo: si no existe lo crea vacio sin truncar el que ya esta.
    std::string path = paths::config(clans_path);
    std::ofstream(path, std::ios::binary | std::ios::app);
}

std::vector<ClanRecord> ClanPersistence::load_all() const {
    std::vector<ClanRecord> records;
    std::string path = paths::config(clans_path);
    std::ifstream in(path, std::ios::binary);
    if (!in) return records;

    ClanRecord rec;
    while (in.read(reinterpret_cast<char*>(&rec), sizeof(rec))) {
        records.push_back(rec);
    }
    return records;
}

void ClanPersistence::save_all(const std::vector<ClanRecord>& records) const {
    // trunc: reescribe el archivo entero con el set actual de clanes.
    std::string path = paths::config(clans_path);
    std::ofstream out(path, std::ios::binary | std::ios::trunc);
    if (!out) throw std::runtime_error("ClanPersistence: no se pudo abrir " + path);
    for (const ClanRecord& rec : records) {
        out.write(reinterpret_cast<const char*>(&rec), sizeof(rec));
    }
    out.flush();
}
