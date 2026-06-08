#include "clan_persistence.h"

#include <sys/stat.h>
#include <sys/types.h>

#include <fstream>
#include <stdexcept>

#include "../../common/constants/game_constants.h"

ClanPersistence::ClanPersistence(const std::string& data_dir) {
    clans_path = data_dir + PERSIST_CLANS_FILE;
    ensure_file(data_dir);
}

void ClanPersistence::ensure_file(const std::string& data_dir) const {
    // Crea el directorio si no existe (modo 0755; ignora EEXIST).
    ::mkdir(data_dir.c_str(), 0755);
    // "Toca" el archivo: si no existe lo crea vacio sin truncar el que ya esta.
    std::ofstream(clans_path, std::ios::binary | std::ios::app);
}

std::vector<ClanRecord> ClanPersistence::load_all() const {
    std::vector<ClanRecord> records;
    std::ifstream in(clans_path, std::ios::binary);
    if (!in) return records;

    ClanRecord rec;
    while (in.read(reinterpret_cast<char*>(&rec), sizeof(rec))) {
        records.push_back(rec);
    }
    return records;
}

void ClanPersistence::save_all(const std::vector<ClanRecord>& records) const {
    // trunc: reescribe el archivo entero con el set actual de clanes.
    std::ofstream out(clans_path, std::ios::binary | std::ios::trunc);
    if (!out) throw std::runtime_error("ClanPersistence: no se pudo abrir " + clans_path);
    for (const ClanRecord& rec : records) {
        out.write(reinterpret_cast<const char*>(&rec), sizeof(rec));
    }
    out.flush();
}
