#include "player_persistence.h"

#include <filesystem>

#include <cstdio>
#include <cstring>
#include <fstream>
#include <stdexcept>

#include "../../common/constants/game_constants.h"
#include "paths.h"

PlayerPersistence::PlayerPersistence(const std::string& data_dir) {
    players_path = data_dir + PERSIST_PLAYERS_FILE;
    index_path   = data_dir + PERSIST_INDEX_FILE;
    ensure_files(data_dir);
    load_index();
}

void PlayerPersistence::ensure_files(const std::string& data_dir) {
    std::filesystem::create_directories(paths::config(data_dir));

    std::string path = paths::config(players_path);
    std::ofstream(path, std::ios::binary | std::ios::app);
    path = paths::config(index_path);
    std::ofstream(path, std::ios::binary | std::ios::app);
}

void PlayerPersistence::load_index() {
    index.clear();
    std::string path = paths::config(index_path);
    std::ifstream in(path, std::ios::binary);
    if (!in) return;

    IndexEntry entry;
    while (in.read(reinterpret_cast<char*>(&entry), sizeof(entry))) {
        std::string name(entry.name, ::strnlen(entry.name, PERSIST_NAME_MAX));

        index[name] = entry.offset;
    }
}

bool PlayerPersistence::has_player(const std::string& name) const {
    return index.find(name) != index.end();
}

bool PlayerPersistence::load(const std::string& name, PlayerRecord& out) const {
    auto it = index.find(name);
    if (it == index.end()) return false;

    std::string path = paths::config(players_path);
    std::ifstream in(path, std::ios::binary);
    if (!in) return false;
    in.seekg(static_cast<std::streamoff>(it->second), std::ios::beg);
    in.read(reinterpret_cast<char*>(&out), sizeof(PlayerRecord));
    return static_cast<bool>(in);
}

void PlayerPersistence::append_index_entry(const std::string& name, uint64_t offset) {
    IndexEntry entry;
    std::memset(&entry, 0, sizeof(entry));
    std::strncpy(entry.name, name.c_str(), PERSIST_NAME_MAX - 1);
    entry.offset = offset;

    std::string path = paths::config(index_path);
    std::ofstream out(path, std::ios::binary | std::ios::app);
    out.write(reinterpret_cast<const char*>(&entry), sizeof(entry));
    out.flush();
}

void PlayerPersistence::save(const std::string& name, const PlayerRecord& rec) {
    std::string path = paths::config(players_path);
    auto it = index.find(name);
    if (it != index.end()) {
        std::fstream out(path, std::ios::binary | std::ios::in | std::ios::out);
        if (!out) throw std::runtime_error("PlayerPersistence: no se pudo abrir " + path);
        out.seekp(static_cast<std::streamoff>(it->second), std::ios::beg);
        out.write(reinterpret_cast<const char*>(&rec), sizeof(PlayerRecord));
        out.flush();
        return;
    }

    std::ofstream out(path, std::ios::binary | std::ios::app);
    if (!out) throw std::runtime_error("PlayerPersistence: no se pudo abrir " + path);
    out.seekp(0, std::ios::end);
    uint64_t offset = static_cast<uint64_t>(out.tellp());
    out.write(reinterpret_cast<const char*>(&rec), sizeof(PlayerRecord));
    out.flush();

    index[name] = offset;
    append_index_entry(name, offset);
}
