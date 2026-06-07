#ifndef PLAYER_PERSISTENCE_H_
#define PLAYER_PERSISTENCE_H_

#include <cstdint>
#include <map>
#include <string>

#include "player_record.h"

// I/O binario de jugadores sobre los DOS archivos que pide la consigna:
//  - players.dat: arreglo de PlayerRecord de tamaño FIJO. El record en el offset
//    O se lee/escribe con un seek directo (no se carga el archivo entero).
//  - index.dat:   diccionario nombre -> offset en players.dat. Es chico, asi que
//    se carga ENTERO a memoria (std::map) al iniciar y se appendea en disco.
//
// Encapsula ambos archivos: nadie mas toca el disco. El game loop la usa via
// has_player/load/save (sin locks: corre en el unico hilo que muta el mundo).
class PlayerPersistence {
public:
    // Crea el directorio y los archivos si no existen y carga el indice a memoria.
    explicit PlayerPersistence(const std::string& data_dir);

    // ¿El nombre existe? Consulta SOLO el indice en memoria (no toca players.dat).
    bool has_player(const std::string& name) const;

    // Lee el record de `name` (seek a su offset + read). Devuelve false si no
    // existe en el indice.
    bool load(const std::string& name, PlayerRecord& out) const;

    // Persiste el record. Si el nombre ya esta en el indice -> reescribe in-place
    // en su offset; si no -> appendea al final de players.dat, guarda el offset
    // en el indice (memoria) y appendea la entrada a index.dat.
    void save(const std::string& name, const PlayerRecord& rec);

private:
    std::string players_path;
    std::string index_path;
    std::map<std::string, uint64_t> index;  // nombre -> offset en players.dat

    // Crea el directorio (si falta) y toca ambos archivos para que existan.
    void ensure_files(const std::string& data_dir);
    // Lee index.dat entero al std::map.
    void load_index();
    // Appendea una entrada {name, offset} a index.dat.
    void append_index_entry(const std::string& name, uint64_t offset);
};

#endif  // PLAYER_PERSISTENCE_H_
