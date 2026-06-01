// ============================================================================
// Tests del formato binario de mapas . GoogleTest, SIN Qt.
//
//  - RoundTrip: save -> load reproduce dims, tilesets y capas celda a celda
//    (incluye gids > 255 para ejercitar el almacenamiento uint16).
//  - RejectsBadMagic: un archivo con magic invalido lanza.
//  - ForwardCompatSkipsUnknownSection: una seccion de tipo desconocido se
//    saltea (forward-compat) y las conocidas siguen leyendose.
//  - CollisionMatchesMapLoader: cargar un .toml con MapLoader, volcarlo a .bin,
//    recargar con BinaryMapLoader y comparar is_collidable celda por celda ->
//    prueba que el server obtiene el mismo mundo desde un .bin.
// ============================================================================

#include <cstdint>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "binaryMapFormat.h"
#include "binaryMapLoader.h"
#include "binaryMapSaver.h"
#include "mapLoader.h"

namespace {

// Path temporal unico para los .bin de prueba (se borra al final de cada test).
std::string temp_bin_path(const std::string& tag) {
    return (std::filesystem::temp_directory_path() /
            ("aomap_test_" + tag + ".bin")).string();
}

// Construye dos capas WIDTH-ish x HEIGHT-ish con gids conocidos, incluyendo
// valores > 255 para verificar que el uint16 no se trunca.
std::vector<MapLayerData> make_layers(int width, int height) {
    MapLayerData ground;
    ground.name = "ground";
    MapLayerData buildings;
    buildings.name = "buildings";
    for (int y = 0; y < height; ++y) {
        std::vector<int> g_row, b_row;
        for (int x = 0; x < width; ++x) {
            g_row.push_back((x + y) % 2 == 0 ? 1 : 0);
            // gid grande (> 255) para probar el rango uint16.
            b_row.push_back((x == y) ? 300 + x : 0);
        }
        ground.data.push_back(std::move(g_row));
        buildings.data.push_back(std::move(b_row));
    }
    return {ground, buildings};
}

std::vector<Tileset> make_tilesets() {
    Tileset sand{"sand", "sand.png", 1, 1, 1, false};
    // firstgid 2.. cubre hasta 257; el gid 300 de arriba cae fuera adrede (queda
    // sin TileDef), lo cual es valido: find_tile devuelve nullptr para gids no
    // registrados. Para el round-trip de datos crudos no importa.
    Tileset buildings{"buildings", "buildings.png", 16, 256, 2, true};
    return {sand, buildings};
}

}  // namespace

TEST(BinaryMap, RoundTrip) {
    const int tile_size = 64, width = 30, height = 16;
    auto tilesets = make_tilesets();
    auto layers = make_layers(width, height);

    std::string path = temp_bin_path("roundtrip");
    ASSERT_NO_THROW(
        BinaryMapSaver::save(path, tile_size, width, height, tilesets, layers));

    BinaryMapLoader loader;
    ASSERT_NO_THROW(loader.load(path));

    EXPECT_EQ(loader.get_tile_size(), tile_size);
    EXPECT_EQ(loader.get_width(), width);
    EXPECT_EQ(loader.get_height(), height);

    // Tilesets: comparar campos (file_path se resuelve a absoluto al cargar, asi
    // que se compara solo el basename via "termina con").
    ASSERT_EQ(loader.get_tilesets().size(), tilesets.size());
    for (size_t i = 0; i < tilesets.size(); ++i) {
        const Tileset& got = loader.get_tilesets()[i];
        const Tileset& exp = tilesets[i];
        EXPECT_EQ(got.name, exp.name);
        EXPECT_EQ(got.columns, exp.columns);
        EXPECT_EQ(got.tile_count, exp.tile_count);
        EXPECT_EQ(got.firstgid, exp.firstgid);
        EXPECT_EQ(got.collidable, exp.collidable);
        // file_path absoluto termina con el relativo original.
        EXPECT_GE(got.file_path.size(), exp.file_path.size());
        EXPECT_EQ(got.file_path.substr(got.file_path.size() - exp.file_path.size()),
                  exp.file_path);
    }

    // Capas celda a celda (incluye los gids > 255).
    ASSERT_EQ(loader.get_layers().size(), layers.size());
    for (size_t l = 0; l < layers.size(); ++l) {
        EXPECT_EQ(loader.get_layers()[l].name, layers[l].name);
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                EXPECT_EQ(loader.get_layers()[l].data[y][x], layers[l].data[y][x])
                    << "capa " << l << " celda (" << x << "," << y << ")";
            }
        }
    }

    std::remove(path.c_str());
}

TEST(BinaryMap, RejectsBadMagic) {
    std::string path = temp_bin_path("badmagic");
    {
        std::ofstream f(path, std::ios::binary);
        const char garbage[16] = {'N', 'O', 'T', 'A', 'M', 'A', 'P', '!',
                                  0, 1, 1, 2, 0, 0, 0, 0};
        f.write(garbage, sizeof(garbage));
    }
    BinaryMapLoader loader;
    EXPECT_THROW(loader.load(path), std::runtime_error);
    std::remove(path.c_str());
}

TEST(BinaryMap, ForwardCompatSkipsUnknownSection) {
    // Construimos a mano un .bin con una seccion desconocida (0x7FFF) entre META
    // y LAYERS. El loader debe saltearla y leer las conocidas igual.
    std::string path = temp_bin_path("forwardcompat");

    auto put_u16 = [](std::vector<uint8_t>& b, uint16_t v) {
        b.push_back((v >> 8) & 0xFF); b.push_back(v & 0xFF);
    };
    auto put_u32 = [](std::vector<uint8_t>& b, uint32_t v) {
        b.push_back((v >> 24) & 0xFF); b.push_back((v >> 16) & 0xFF);
        b.push_back((v >> 8) & 0xFF);  b.push_back(v & 0xFF);
    };

    std::vector<uint8_t> buf;
    // Header: MAGIC + version + endianness + section_count = 4 (META, UNKNOWN,
    // TILESETS, LAYERS).
    for (char c : BinaryMapFormat::MAGIC) buf.push_back(static_cast<uint8_t>(c));
    put_u16(buf, BinaryMapFormat::FORMAT_VERSION);
    put_u16(buf, BinaryMapFormat::ENDIANNESS_MARKER);
    put_u32(buf, 4);

    const int width = 2, height = 2;
    // META.
    put_u16(buf, static_cast<uint16_t>(BinaryMapFormat::Section::META));
    put_u32(buf, 12);
    put_u32(buf, 64); put_u32(buf, width); put_u32(buf, height);
    // UNKNOWN (tipo reservado/futuro): 3 bytes de payload basura.
    put_u16(buf, 0x7FFF);
    put_u32(buf, 3);
    buf.push_back(0xDE); buf.push_back(0xAD); buf.push_back(0xBE);
    // TILESETS: 0 tilesets.
    put_u16(buf, static_cast<uint16_t>(BinaryMapFormat::Section::TILESETS));
    put_u32(buf, 4);
    put_u32(buf, 0);
    // LAYERS: 1 capa "x" con matriz 2x2 (gids 0,1,2,3).
    {
        std::vector<uint8_t> payload;
        put_u32(payload, 1);            // layer_count
        put_u16(payload, 1); payload.push_back('x');  // name "x"
        uint16_t gid = 0;
        for (int y = 0; y < height; ++y)
            for (int x = 0; x < width; ++x) put_u16(payload, gid++);
        put_u16(buf, static_cast<uint16_t>(BinaryMapFormat::Section::LAYERS));
        put_u32(buf, static_cast<uint32_t>(payload.size()));
        buf.insert(buf.end(), payload.begin(), payload.end());
    }

    { std::ofstream f(path, std::ios::binary);
      f.write(reinterpret_cast<const char*>(buf.data()), buf.size()); }

    BinaryMapLoader loader;
    ASSERT_NO_THROW(loader.load(path));
    EXPECT_EQ(loader.get_width(), width);
    EXPECT_EQ(loader.get_height(), height);
    ASSERT_EQ(loader.get_layers().size(), 1u);
    EXPECT_EQ(loader.get_layers()[0].name, "x");
    EXPECT_EQ(loader.get_layers()[0].data[0][0], 0);
    EXPECT_EQ(loader.get_layers()[0].data[1][1], 3);
    std::remove(path.c_str());
}

TEST(BinaryMap, CollisionMatchesMapLoader) {
    // Cargar un mapa real en TOML, volcarlo a .bin y verificar que el mundo de
    // colision sea identico celda a celda. Es la garantia de que el server
    // obtiene el mismo mundo desde un .bin que desde un .toml.
    const std::string toml = "data/maps/desert/map.toml";
    MapLoader toml_loader;
    try {
        toml_loader.load(toml);
    } catch (const std::exception& e) {
        GTEST_SKIP() << "No se pudo cargar " << toml << " (se corre desde otro cwd?): "
                     << e.what();
    }

    std::string path = temp_bin_path("collision");
    ASSERT_NO_THROW(BinaryMapSaver::save(
        path, toml_loader.get_tile_size(), toml_loader.get_width(),
        toml_loader.get_height(), toml_loader.get_tilesets(),
        toml_loader.get_layers()));

    BinaryMapLoader bin_loader;
    ASSERT_NO_THROW(bin_loader.load(path));

    ASSERT_EQ(bin_loader.get_width(), toml_loader.get_width());
    ASSERT_EQ(bin_loader.get_height(), toml_loader.get_height());
    for (int y = 0; y < toml_loader.get_height(); ++y) {
        for (int x = 0; x < toml_loader.get_width(); ++x) {
            EXPECT_EQ(bin_loader.is_collidable(x, y), toml_loader.is_collidable(x, y))
                << "colision difiere en (" << x << "," << y << ")";
        }
    }
    std::remove(path.c_str());
}
