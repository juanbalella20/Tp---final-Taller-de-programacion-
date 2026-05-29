#ifndef MAP_LOADER_H
#define MAP_LOADER_H

#include <map>
#include <string>
#include <unordered_map>
#include <vector>

#include "constants/game_constants.h"
#include "../vendored/tomlplusplus/toml.hpp"
#include <filesystem>



struct Tileset {
    std::string name;
    std::string file_path;  // path absoluto
    int columns;            // tiles por fila en el spritesheet
    int tile_count;         // total de tiles utiles
    int firstgid;           // primer ID global cubierto por este tileset
    bool collidable;        // si true, todos los tiles del set son collidables por defecto
};

// Definicion de un tile concreto
struct TileDef {
    int id;                  // ID global (incluye firstgid)
    int tileset_index;       // indice en MapData::tilesets
    int local_index;         // id - firstgid (posicion dentro del tileset)
    bool collidable;
};

// Una capa de mapa: matriz [row][col] de IDs globales. id=0 = celda vacia.
struct MapLayerData {
    std::string name;
    std::vector<std::vector<int>> data; // array de arrays
};

class MapLoader {
    private: 
        int tile_size = 0;
        int width = 0; 
        int height = 0;
        std::vector<Tileset> tilesets;
        std::unordered_map<int, TileDef> tiles;   // id global -> tiledef
        std::vector<MapLayerData> layers;
        std::map<std::string, positionCoord> spawns;

        /*
         * Recorre la tabla (array) de tilesets del TOML.
         * Construye un tipo Tileset
         * Lo pushea al vector de tilesets de MapLoader
        */
        void parse_tilesets(const toml::table& tbl,
                            const std::filesystem::path& baseDir);

        // Recorre el vector de tileset y arma el mapa de tiles <id,tiles>
        void build_tile_index();

        /* Traduce las capas del TOML a tipo MapLayerData
         * Las guarda en el vector de layers
         * valda que cada matriz tenga el correcto width y height
         * y que el contenido sea numérico
         */
        void parse_layers(const toml::table& tbl);
        // Traduce los spawns del TOML y los almacena el mapa de spawns
        void parse_spawns(const toml::table& tbl);
     

    public:
        int get_tile_size() const;
        int get_width() const;
        int get_height() const;
        const std::map<std::string, positionCoord>& get_spawns() const;
        const std::vector<Tileset>& get_tilesets() const;
        const std::vector<MapLayerData>& get_layers() const;


        // True si la celda esta fuera del mapa
        // True si alguna capa tiene en (x,y) un tile colidable
        bool is_collidable(int x, int y) const;

        // Devuelve nullptr si el id es 0 o no esta registrado en ningun tileset.
        const TileDef* find_tile(int id) const;

        /*
         * Abre y parsea el archivo
         * Lee la metada del mapa 
         * Llena los campos de MapLoader: tilesets, tiles, layers, spwans
         */
        void load(const std::string& tomlPath);

        
};

#endif
