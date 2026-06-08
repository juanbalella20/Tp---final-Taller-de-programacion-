#ifndef CLAN_PERSISTENCE_H_
#define CLAN_PERSISTENCE_H_

#include <string>
#include <vector>

#include "player_record.h"

// I/O binario de clanes sobre un unico archivo (clans.dat): un arreglo de
// ClanRecord de tamaño FIJO. A diferencia de players.dat (acceso por offset via
// index.dat), los clanes se cargan/guardan EN BLOQUE: son pocos y siempre que
// muta uno conviene reescribir el set entero. No hay archivo de indice: la clave
// (clan_name) vive dentro de cada record.
//
// El game loop es el unico hilo que muta el mundo, asi que persiste sin locks.
class ClanPersistence {
public:
    // Crea el directorio y el archivo si no existen.
    explicit ClanPersistence(const std::string& data_dir);

    // Lee clans.dat entero. Devuelve todos los records (vacio si no hay clanes).
    std::vector<ClanRecord> load_all() const;

    // Reescribe clans.dat con exactamente estos records (truncando lo previo).
    void save_all(const std::vector<ClanRecord>& records) const;

private:
    std::string clans_path;

    // Crea el directorio (si falta) y toca el archivo para que exista.
    void ensure_file(const std::string& data_dir) const;
};

#endif  // CLAN_PERSISTENCE_H_
