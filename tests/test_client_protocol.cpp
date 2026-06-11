

#include <gtest/gtest.h>

#include <cstdint>
#include <vector>

#include "serializer.h"         
#include "serverDeserializer.h" 
#include "clientCmd.h"
#include "protocol_constants.h"

namespace {


ClientCmd round_trip(const std::vector<uint8_t>& bytes) {
    EXPECT_GE(bytes.size(), static_cast<size_t>(LEN_HEADER));
    uint8_t type = bytes[0];
    uint16_t payload_len = (static_cast<uint16_t>(bytes[1]) << 8) | bytes[2];
    EXPECT_EQ(bytes.size(), static_cast<size_t>(LEN_HEADER) + payload_len);

    std::vector<uint8_t> payload(bytes.begin() + LEN_HEADER, bytes.end());

    ServerDeserializer deserializer;
    ClientCmd out;
    deserializer.deserialize_cmd(type, payload, out);
    return out;
}

}  // namespace

// ─── MOVE: el comando mas simple, valida toda la cadena de build ──────────────

TEST(ClientProtocolMove, RoundTripPreservaDireccion) {
    Serializer serializer;

    ClientCmd cmd;
    cmd.set_message_type(MSG_MOVE);
    cmd.set_direction(DIR_EAST);

    std::vector<uint8_t> bytes = serializer.serialize_cmd(cmd);
    ClientCmd out = round_trip(bytes);

    EXPECT_EQ(out.get_message_type(), MSG_MOVE);
    EXPECT_EQ(out.get_direction(), DIR_EAST);
}

TEST(ClientProtocolMove, FormatoBinarioExacto) {
    Serializer serializer;

    ClientCmd cmd;
    cmd.set_message_type(MSG_MOVE);
    cmd.set_direction(DIR_SOUTH);

    std::vector<uint8_t> bytes = serializer.serialize_cmd(cmd);

    // [type=MSG_MOVE][payload_len=1 BE => 0x00 0x01][direction=DIR_SOUTH]
    ASSERT_EQ(bytes.size(), 4u);
    EXPECT_EQ(bytes[0], MSG_MOVE);
    EXPECT_EQ(bytes[1], 0x00);
    EXPECT_EQ(bytes[2], 0x01);
    EXPECT_EQ(bytes[3], DIR_SOUTH);
}

TEST(ClientProtocolMove, TodasLasDirecciones) {
    Serializer serializer;
    for (Direction dir : {DIR_NORTH, DIR_SOUTH, DIR_EAST, DIR_WEST}) {
        ClientCmd cmd;
        cmd.set_message_type(MSG_MOVE);
        cmd.set_direction(dir);

        ClientCmd out = round_trip(serializer.serialize_cmd(cmd));
        EXPECT_EQ(out.get_direction(), dir);
    }
}

// ─── REGISTER / LOGIN: strings con prefijo de largo + raza + clase ────────────

TEST(ClientProtocolRegister, RoundTripPreservaTodosLosCampos) {
    Serializer serializer;

    ClientCmd cmd;
    cmd.set_message_type(MSG_REGISTER);
    cmd.set_player_name("Gandalf");
    cmd.set_password("secreto123");
    cmd.set_race("elf");
    cmd.set_class("wizard");

    ClientCmd out = round_trip(serializer.serialize_cmd(cmd));

    EXPECT_EQ(out.get_message_type(), MSG_REGISTER);
    EXPECT_EQ(out.get_player_name(), "Gandalf");
    EXPECT_EQ(out.get_password(), "secreto123");
    EXPECT_EQ(out.get_race(), "elf");
    EXPECT_EQ(out.get_class(), "wizard");
}

TEST(ClientProtocolRegister, FormatoBinarioConPrefijosDeLargo) {
    Serializer serializer;

    ClientCmd cmd;
    cmd.set_message_type(MSG_REGISTER);
    cmd.set_player_name("ab");      // name_len = 2
    cmd.set_password("xyz");        // pass_len = 3
    cmd.set_race("human");          // RACE_HUMAN = 0x00
    cmd.set_class("warrior");       // WARRIOR    = 0x03

    std::vector<uint8_t> b = serializer.serialize_cmd(cmd);

    // payload = [2]['a']['b'][3]['x']['y']['z'][0x00][0x03] = 9 bytes
    ASSERT_EQ(b.size(), static_cast<size_t>(LEN_HEADER) + 9u);
    EXPECT_EQ(b[0], MSG_REGISTER);
    EXPECT_EQ(b[1], 0x00);
    EXPECT_EQ(b[2], 0x09);           // payload_len big-endian
    EXPECT_EQ(b[3], 2);              // name_len
    EXPECT_EQ(b[4], 'a');
    EXPECT_EQ(b[5], 'b');
    EXPECT_EQ(b[6], 3);              // pass_len
    EXPECT_EQ(b[7], 'x');
    EXPECT_EQ(b[8], 'y');
    EXPECT_EQ(b[9], 'z');
    EXPECT_EQ(b[10], RACE_HUMAN);
    EXPECT_EQ(b[11], WARRIOR);
}

TEST(ClientProtocolLogin, RoundTripPreservaNombreYPassword) {
    Serializer serializer;

    ClientCmd cmd;
    cmd.set_message_type(MSG_LOGIN);
    cmd.set_player_name("Frodo");
    cmd.set_password("anillo");

    ClientCmd out = round_trip(serializer.serialize_cmd(cmd));

    EXPECT_EQ(out.get_message_type(), MSG_LOGIN);
    EXPECT_EQ(out.get_player_name(), "Frodo");
    EXPECT_EQ(out.get_password(), "anillo");
}

// ─── ATTACK / SELECT: coordenadas uint16 big-endian ──────────────────────────

TEST(ClientProtocolCoords, AttackRoundTripPreservaCoordenadas) {
    Serializer serializer;

    ClientCmd cmd;
    cmd.set_message_type(MSG_ATTACK);
    cmd.set_coord_x(258);   // 0x0102
    cmd.set_coord_y(513);   // 0x0201

    ClientCmd out = round_trip(serializer.serialize_cmd(cmd));

    EXPECT_EQ(out.get_message_type(), MSG_ATTACK);
    EXPECT_EQ(out.get_coord_x(), 258);
    EXPECT_EQ(out.get_coord_y(), 513);
}

TEST(ClientProtocolCoords, FormatoBinarioBigEndian) {
    Serializer serializer;

    ClientCmd cmd;
    cmd.set_message_type(MSG_SELECT);
    cmd.set_coord_x(258);   // 0x0102 -> bytes 0x01 0x02
    cmd.set_coord_y(513);   // 0x0201 -> bytes 0x02 0x01

    std::vector<uint8_t> b = serializer.serialize_cmd(cmd);

    ASSERT_EQ(b.size(), static_cast<size_t>(LEN_HEADER) + 4u);
    EXPECT_EQ(b[0], MSG_SELECT);
    EXPECT_EQ(b[1], 0x00);
    EXPECT_EQ(b[2], 0x04);           // payload_len = 4
    EXPECT_EQ(b[3], 0x01);           // x high byte
    EXPECT_EQ(b[4], 0x02);           // x low byte
    EXPECT_EQ(b[5], 0x02);           // y high byte
    EXPECT_EQ(b[6], 0x01);           // y low byte
}

// ─── item_id: comandos que llevan un id de item en texto ──────────────────────

TEST(ClientProtocolItemId, EquipRoundTrip) {
    Serializer serializer;

    ClientCmd cmd;
    cmd.set_message_type(MSG_EQUIP);
    cmd.set_item_id("espada_larga");

    ClientCmd out = round_trip(serializer.serialize_cmd(cmd));

    EXPECT_EQ(out.get_message_type(), MSG_EQUIP);
    EXPECT_EQ(out.get_item_id(), "espada_larga");
}

TEST(ClientProtocolItemId, BuySellThrowComparteFormato) {
    Serializer serializer;
    for (MessageType type : {MSG_BUY, MSG_SELL, MSG_THROW, MSG_DEPOSIT, MSG_RETIRE, MSG_USE_ITEM}) {
        ClientCmd cmd;
        cmd.set_message_type(type);
        cmd.set_item_id("pocion_roja");

        ClientCmd out = round_trip(serializer.serialize_cmd(cmd));
        EXPECT_EQ(out.get_message_type(), type);
        EXPECT_EQ(out.get_item_id(), "pocion_roja");
    }
}

// ─── gold: uint32 big-endian ──────────────────────────────────────────────────

TEST(ClientProtocolGold, DepGoldRoundTrip) {
    Serializer serializer;

    ClientCmd cmd;
    cmd.set_message_type(MSG_DEP_GOLD);
    cmd.set_gold(1000000);

    ClientCmd out = round_trip(serializer.serialize_cmd(cmd));

    EXPECT_EQ(out.get_message_type(), MSG_DEP_GOLD);
    EXPECT_EQ(out.get_gold(), 1000000u);
}

TEST(ClientProtocolGold, FormatoBinarioBigEndian) {
    Serializer serializer;

    ClientCmd cmd;
    cmd.set_message_type(MSG_RET_GOLD);
    cmd.set_gold(0x01020304);

    std::vector<uint8_t> b = serializer.serialize_cmd(cmd);

    ASSERT_EQ(b.size(), static_cast<size_t>(LEN_HEADER) + 4u);
    EXPECT_EQ(b[0], MSG_RET_GOLD);
    EXPECT_EQ(b[1], 0x00);
    EXPECT_EQ(b[2], 0x04);
    EXPECT_EQ(b[3], 0x01);
    EXPECT_EQ(b[4], 0x02);
    EXPECT_EQ(b[5], 0x03);
    EXPECT_EQ(b[6], 0x04);
}

// ─── zona (TELEPORT) ──────────────────────────────────────────────────────────

TEST(ClientProtocolZone, TeleportRoundTrip) {
    Serializer serializer;

    ClientCmd cmd;
    cmd.set_message_type(MSG_TELEPORT);
    cmd.set_zone(ZONE_DUNGEON);

    ClientCmd out = round_trip(serializer.serialize_cmd(cmd));

    EXPECT_EQ(out.get_message_type(), MSG_TELEPORT);
    EXPECT_EQ(out.get_zone(), ZONE_DUNGEON);
}

// ─── clanes: solo target_name ─────────────────────────────────────────────────

TEST(ClientProtocolClan, FoundClanRoundTrip) {
    Serializer serializer;

    ClientCmd cmd;
    cmd.set_message_type(MSG_FOUND_CLAN);
    cmd.set_target_name("LosCaballeros");

    ClientCmd out = round_trip(serializer.serialize_cmd(cmd));

    EXPECT_EQ(out.get_message_type(), MSG_FOUND_CLAN);
    EXPECT_EQ(out.get_target_name(), "LosCaballeros");
}

// ─── PRIVATE: target_name + chat_content, ambos con prefijo de largo ──────────

TEST(ClientProtocolPrivate, RoundTripPreservaDestinatarioYMensaje) {
    Serializer serializer;

    ClientCmd cmd;
    cmd.set_message_type(MSG_PRIVATE);
    cmd.set_target_name("Aragorn");
    cmd.set_chat_content("nos vemos en Rivendel");

    ClientCmd out = round_trip(serializer.serialize_cmd(cmd));

    EXPECT_EQ(out.get_message_type(), MSG_PRIVATE);
    EXPECT_EQ(out.get_target_name(), "Aragorn");
    EXPECT_EQ(out.get_chat_content(), "nos vemos en Rivendel");
}

// ─── comandos sin payload: solo header, payload_len = 0 ───────────────────────

TEST(ClientProtocolNoPayload, SoloHeaderConPayloadCero) {
    Serializer serializer;
    for (MessageType type : {MSG_MEDITATE, MSG_RESURRECT, MSG_CURE, MSG_LIST,
                             MSG_SELF_CAST, MSG_CHEAT_KILL, MSG_LEFT_CLAN}) {
        ClientCmd cmd;
        cmd.set_message_type(type);

        std::vector<uint8_t> b = serializer.serialize_cmd(cmd);
        ASSERT_EQ(b.size(), static_cast<size_t>(LEN_HEADER));
        EXPECT_EQ(b[0], type);
        EXPECT_EQ(b[1], 0x00);
        EXPECT_EQ(b[2], 0x00);          // payload_len = 0

        ClientCmd out = round_trip(b);
        EXPECT_EQ(out.get_message_type(), type);
    }
}

// ─── casos borde ──────────────────────────────────────────────────────────────

TEST(ClientProtocolEdge, NombreVacio) {
    Serializer serializer;

    ClientCmd cmd;
    cmd.set_message_type(MSG_LOGIN);
    cmd.set_player_name("");
    cmd.set_password("");

    ClientCmd out = round_trip(serializer.serialize_cmd(cmd));
    EXPECT_EQ(out.get_player_name(), "");
    EXPECT_EQ(out.get_password(), "");
}

TEST(ClientProtocolEdge, NombreLargoMaximo255) {
    Serializer serializer;
    std::string nombre(255, 'A');   // limite del campo de largo de 1 byte

    ClientCmd cmd;
    cmd.set_message_type(MSG_FOUND_CLAN);
    cmd.set_target_name(nombre);

    ClientCmd out = round_trip(serializer.serialize_cmd(cmd));
    EXPECT_EQ(out.get_target_name(), nombre);
    EXPECT_EQ(out.get_target_name().size(), 255u);
}

TEST(ClientProtocolEdge, CoordenadasEnElMaximoUint16) {
    Serializer serializer;

    ClientCmd cmd;
    cmd.set_message_type(MSG_ATTACK);
    cmd.set_coord_x(65535);
    cmd.set_coord_y(0);

    ClientCmd out = round_trip(serializer.serialize_cmd(cmd));
    EXPECT_EQ(out.get_coord_x(), 65535);
    EXPECT_EQ(out.get_coord_y(), 0);
}

TEST(ClientProtocolEdge, GoldCero) {
    Serializer serializer;

    ClientCmd cmd;
    cmd.set_message_type(MSG_DEP_GOLD);
    cmd.set_gold(0);

    ClientCmd out = round_trip(serializer.serialize_cmd(cmd));
    EXPECT_EQ(out.get_gold(), 0u);
}
