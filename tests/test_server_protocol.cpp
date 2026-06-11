// Tests del protocolo de comunicacion (servidor -> cliente).
//
// Mismo enfoque que test_client_protocol.cpp: round-trip puro sin sockets. Se
// serializa un GameMsg con el ServerSerializer y se vuelve a parsear con el
// ClientDeserializer, verificando que los campos sobreviven, mas asserts sobre
// los bytes exactos del formato binario.

#include <gtest/gtest.h>

#include <cstdint>
#include <string>
#include <vector>

#include "serverSerializer.h"  // server/serializer/serverSerializer.h
#include "deserializer.h"      // client/serializers/deserializer.h
#include "gameMsg.h"
#include "protocol_constants.h"
#include "game_constants.h"
#include "item_info.h"
#include "npc_info.h"
#include "playerinfo.h"
#include "item_floor_info.h"

namespace {

// Replica ClientProtocol::receive_event sobre un buffer en memoria: separa el
// header [type:1B][payload_len:2B BE] y delega al ClientDeserializer.
GameMsg round_trip(const std::vector<uint8_t>& bytes) {
    EXPECT_GE(bytes.size(), static_cast<size_t>(LEN_HEADER));
    uint8_t type = bytes[0];
    uint16_t payload_len = (static_cast<uint16_t>(bytes[1]) << 8) | bytes[2];
    EXPECT_EQ(bytes.size(), static_cast<size_t>(LEN_HEADER) + payload_len);

    std::vector<uint8_t> payload(bytes.begin() + LEN_HEADER, bytes.end());

    ClientDeserializer deserializer;
    GameMsg out(type);
    deserializer.deserialize_cmd(type, payload, out);
    return out;
}

}  // namespace

// ─── valores escalares uint32 big-endian (HP / XP / MANA / GOLD) ──────────────

TEST(ServerProtocolValue, HpRoundTrip) {
    ServerSerializer serializer;

    GameMsg msg(MSG_HP);
    msg.set_hp(1234567);

    GameMsg out = round_trip(serializer.serialize_cmd(msg));
    EXPECT_EQ(out.get_type(), MSG_HP);
    EXPECT_EQ(out.get_hp(), 1234567u);
}

TEST(ServerProtocolValue, GoldXpManaRoundTrip) {
    ServerSerializer serializer;

    GameMsg gold(MSG_GOLD);
    gold.set_gold(500);
    EXPECT_EQ(round_trip(serializer.serialize_cmd(gold)).get_gold(), 500u);

    GameMsg xp(MSG_XP);
    xp.set_xp(99999);
    EXPECT_EQ(round_trip(serializer.serialize_cmd(xp)).get_xp(), 99999u);

    GameMsg mana(MSG_MANA);
    mana.set_mana(42);
    EXPECT_EQ(round_trip(serializer.serialize_cmd(mana)).get_mana(), 42u);
}

TEST(ServerProtocolValue, FormatoBinarioBigEndian) {
    ServerSerializer serializer;

    GameMsg msg(MSG_HP);
    msg.set_hp(0x0A0B0C0D);

    std::vector<uint8_t> b = serializer.serialize_cmd(msg);
    ASSERT_EQ(b.size(), static_cast<size_t>(LEN_HEADER) + 4u);
    EXPECT_EQ(b[0], MSG_HP);
    EXPECT_EQ(b[1], 0x00);
    EXPECT_EQ(b[2], 0x04);
    EXPECT_EQ(b[3], 0x0A);
    EXPECT_EQ(b[4], 0x0B);
    EXPECT_EQ(b[5], 0x0C);
    EXPECT_EQ(b[6], 0x0D);
}

// ─── MSG_MOVE (server): direction + name + race + coords ──────────────────────

TEST(ServerProtocolMove, RoundTripPreservaCampos) {
    ServerSerializer serializer;

    GameMsg msg(MSG_MOVE);
    msg.set_direction(DIR_WEST);
    msg.set_player_name("Legolas");
    msg.set_race("elf");
    msg.set_coord_x(12);
    msg.set_coord_y(34);

    GameMsg out = round_trip(serializer.serialize_cmd(msg));
    EXPECT_EQ(out.get_type(), MSG_MOVE);
    EXPECT_EQ(out.get_direction(), DIR_WEST);
    EXPECT_EQ(out.get_player_name(), "Legolas");
    EXPECT_EQ(out.get_race(), "elf");
    EXPECT_EQ(out.get_coord_x(), 12);
    EXPECT_EQ(out.get_coord_y(), 34);
}

// ─── MSG_ATTACK (server): coords + damage ─────────────────────────────────────

TEST(ServerProtocolAttack, RoundTripPreservaCoordsYDamage) {
    ServerSerializer serializer;

    GameMsg msg(MSG_ATTACK);
    msg.set_coord_x(100);
    msg.set_coord_y(200);
    msg.set_damage(57);

    GameMsg out = round_trip(serializer.serialize_cmd(msg));
    EXPECT_EQ(out.get_type(), MSG_ATTACK);
    EXPECT_EQ(out.get_coord_x(), 100);
    EXPECT_EQ(out.get_coord_y(), 200);
    EXPECT_EQ(out.get_damage(), 57);
}

// ─── MSG_ZONE_CHANGE: zona + coords de spawn ──────────────────────────────────

TEST(ServerProtocolZone, RoundTripPreservaZonaYSpawn) {
    ServerSerializer serializer;

    GameMsg msg(MSG_ZONE_CHANGE);
    msg.set_zone(ZONE_FOREST);
    msg.set_coord_x(7);
    msg.set_coord_y(9);

    GameMsg out = round_trip(serializer.serialize_cmd(msg));
    EXPECT_EQ(out.get_type(), MSG_ZONE_CHANGE);
    EXPECT_EQ(out.get_zone(), ZONE_FOREST);
    EXPECT_EQ(out.get_coord_x(), 7);
    EXPECT_EQ(out.get_coord_y(), 9);
}

// ─── MSG_UPDATE_LEVEL: level + max_xp + max_hp + max_mana ──────────────────────

TEST(ServerProtocolLevel, RoundTripPreservaTodosLosMaximos) {
    ServerSerializer serializer;

    GameMsg msg(MSG_UPDATE_LEVEL);
    msg.set_level(5);
    msg.set_max_xp(100000);
    msg.set_max_hp(450);
    msg.set_max_mana(320);

    GameMsg out = round_trip(serializer.serialize_cmd(msg));
    EXPECT_EQ(out.get_type(), MSG_UPDATE_LEVEL);
    EXPECT_EQ(out.get_level(), 5);
    EXPECT_EQ(out.get_max_xp(), 100000u);
    EXPECT_EQ(out.get_max_hp(), 450u);
    EXPECT_EQ(out.get_max_mana(), 320u);
}

// ─── texto (MSG_CHAT): sender + content ───────────────────────────────────────

TEST(ServerProtocolText, ChatRoundTrip) {
    ServerSerializer serializer;

    GameMsg msg(MSG_CHAT);
    msg.set_player_name("Sistema");
    msg.set_chat_content("Bienvenido a Argentum");

    GameMsg out = round_trip(serializer.serialize_cmd(msg));
    EXPECT_EQ(out.get_type(), MSG_CHAT);
    EXPECT_EQ(out.get_player_name(), "Sistema");
    EXPECT_EQ(out.get_chat_content(), "Bienvenido a Argentum");
}

TEST(ServerProtocolText, AuthErrorReusaFormatoTexto) {
    ServerSerializer serializer;

    GameMsg msg(MSG_AUTH_ERROR);
    msg.set_player_name("");
    msg.set_chat_content("nombre ya en uso");

    GameMsg out = round_trip(serializer.serialize_cmd(msg));
    EXPECT_EQ(out.get_type(), MSG_AUTH_ERROR);
    EXPECT_EQ(out.get_chat_content(), "nombre ya en uso");
}

// ─── MSG_DEATH / MSG_PLAYER_LEFT: solo name ───────────────────────────────────

TEST(ServerProtocolName, DeathRoundTrip) {
    ServerSerializer serializer;

    GameMsg msg(MSG_DEATH);
    msg.set_player_name("Boromir");

    GameMsg out = round_trip(serializer.serialize_cmd(msg));
    EXPECT_EQ(out.get_type(), MSG_DEATH);
    EXPECT_EQ(out.get_player_name(), "Boromir");
}

// ─── MSG_CONFIRM_SESSION: name + race + class + clan ──────────────────────────

TEST(ServerProtocolConfirmSession, RoundTripPreservaTodo) {
    ServerSerializer serializer;

    GameMsg msg(MSG_CONFIRM_SESSION);
    msg.set_player_name("Gimli");
    msg.set_race("dwarf");
    msg.set_class("warrior");
    msg.set_chat_content("LosEnanos");  // clan propio viaja en chat_content

    GameMsg out = round_trip(serializer.serialize_cmd(msg));
    EXPECT_EQ(out.get_type(), MSG_CONFIRM_SESSION);
    EXPECT_EQ(out.get_player_name(), "Gimli");
    EXPECT_EQ(out.get_race(), "dwarf");
    EXPECT_EQ(out.get_class(), "warrior");
    EXPECT_EQ(out.get_chat_content(), "LosEnanos");
}

// ─── MSG_GOLD bytes exactos (auth/oro) ────────────────────────────────────────

TEST(ServerProtocolUpdateEquip, RoundTripPreservaListas) {
    ServerSerializer serializer;

    GameMsg msg(MSG_UPDATE_EQUIP);
    msg.set_player_name("Eowyn");
    msg.set_equipped(true);
    msg.set_equipped_ids({"espada", "armadura"});
    msg.set_equipped_uids({"101", "102"});

    GameMsg out = round_trip(serializer.serialize_cmd(msg));
    EXPECT_EQ(out.get_type(), MSG_UPDATE_EQUIP);
    EXPECT_EQ(out.get_player_name(), "Eowyn");
    EXPECT_TRUE(out.get_equipped());
    ASSERT_EQ(out.get_equipped_ids().size(), 2u);
    EXPECT_EQ(out.get_equipped_ids()[0], "espada");
    EXPECT_EQ(out.get_equipped_ids()[1], "armadura");
    ASSERT_EQ(out.get_equipped_uids().size(), 2u);
    EXPECT_EQ(out.get_equipped_uids()[0], "101");
    EXPECT_EQ(out.get_equipped_uids()[1], "102");
}

// ─── snapshots: NPCs / items en piso / players ────────────────────────────────

TEST(ServerProtocolNpcsSnapshot, RoundTripVariosNpcs) {
    ServerSerializer serializer;

    GameMsg msg(MSG_NPCS_SNAPSHOT);
    msg.set_npcs({
        NpcInfo("goblin", "Gob1", 3, 4, DIR_NORTH),
        NpcInfo("dragon", "Smaug", 10, 11, DIR_EAST),
    });

    GameMsg out = round_trip(serializer.serialize_cmd(msg));
    ASSERT_EQ(out.get_npcs().size(), 2u);
    EXPECT_EQ(out.get_npcs()[0].type, "goblin");
    EXPECT_EQ(out.get_npcs()[0].name, "Gob1");
    EXPECT_EQ(out.get_npcs()[0].x, 3);
    EXPECT_EQ(out.get_npcs()[0].y, 4);
    EXPECT_EQ(out.get_npcs()[0].direction, DIR_NORTH);
    EXPECT_EQ(out.get_npcs()[1].name, "Smaug");
    EXPECT_EQ(out.get_npcs()[1].direction, DIR_EAST);
}

TEST(ServerProtocolNpcsSnapshot, RoundTripVacio) {
    ServerSerializer serializer;
    GameMsg msg(MSG_NPCS_SNAPSHOT);
    msg.set_npcs({});

    GameMsg out = round_trip(serializer.serialize_cmd(msg));
    EXPECT_TRUE(out.get_npcs().empty());
}

TEST(ServerProtocolItemsSnapshot, RoundTripItemsEnPiso) {
    ServerSerializer serializer;

    GameMsg msg(MSG_ITEMS_SNAPSHOT);
    msg.set_items_on_floor({
        ItemFloorInfo("gold", 1, 2),
        ItemFloorInfo("espada", 5, 6),
    });

    GameMsg out = round_trip(serializer.serialize_cmd(msg));
    ASSERT_EQ(out.get_items_on_floor().size(), 2u);
    EXPECT_EQ(out.get_items_on_floor()[0].type, "gold");
    EXPECT_EQ(out.get_items_on_floor()[0].x, 1);
    EXPECT_EQ(out.get_items_on_floor()[1].type, "espada");
    EXPECT_EQ(out.get_items_on_floor()[1].y, 6);
}

TEST(ServerProtocolPlayersSnapshot, RoundTripConClanYEquip) {
    ServerSerializer serializer;

    PlayerInfo p;
    p.name = "Arwen";
    p.race = "elf";
    p.x = 8;
    p.y = 9;
    p.ghost = false;
    p.equipped_ids = {"baculo"};
    p.clan_name = "Rivendel";

    GameMsg msg(MSG_PLAYERS_SNAPSHOT);
    msg.set_players({p});

    GameMsg out = round_trip(serializer.serialize_cmd(msg));
    ASSERT_EQ(out.get_players().size(), 1u);
    const PlayerInfo& got = out.get_players()[0];
    EXPECT_EQ(got.name, "Arwen");
    EXPECT_EQ(got.race, "elf");
    EXPECT_EQ(got.x, 8);
    EXPECT_EQ(got.y, 9);
    EXPECT_FALSE(got.ghost);
    ASSERT_EQ(got.equipped_ids.size(), 1u);
    EXPECT_EQ(got.equipped_ids[0], "baculo");
    EXPECT_EQ(got.clan_name, "Rivendel");
}

// ─── MSG_INVENTORY: id + name + type + uid(8B BE) ─────────────────────────────

TEST(ServerProtocolInventory, RoundTripPreservaUidYType) {
    ServerSerializer serializer;

    GameMsg msg(MSG_INVENTORY);
    std::vector<ItemInfo> items = {
        ItemInfo("espada", "Espada larga", 0, /*type=*/0, /*uid=*/0x0102030405060708ULL),
        ItemInfo("pocion", "Pocion roja", 0, /*type=*/4, /*uid=*/77ULL),
    };
    msg.set_items(items);

    GameMsg out = round_trip(serializer.serialize_cmd(msg));
    ASSERT_EQ(out.get_items().size(), 2u);
    EXPECT_EQ(out.get_items()[0].id, "espada");
    EXPECT_EQ(out.get_items()[0].name, "Espada larga");
    EXPECT_EQ(out.get_items()[0].type, 0);
    EXPECT_EQ(out.get_items()[0].uid, 0x0102030405060708ULL);
    EXPECT_EQ(out.get_items()[1].uid, 77ULL);
}

// ─── MSG_LIST: catalogo de tienda con precio (4B BE) ──────────────────────────

TEST(ServerProtocolSellerList, RoundTripPreservaPrecio) {
    ServerSerializer serializer;

    GameMsg msg(MSG_LIST);
    msg.set_items({ ItemInfo("escudo", "Escudo de hierro", 250, /*type=*/3, /*uid=*/0) });

    GameMsg out = round_trip(serializer.serialize_cmd(msg));
    ASSERT_EQ(out.get_items().size(), 1u);
    EXPECT_EQ(out.get_items()[0].id, "escudo");
    EXPECT_EQ(out.get_items()[0].price, 250);
    EXPECT_EQ(out.get_items()[0].type, 3);
}

// ─── MSG_SEND_MAP: matriz HEIGHT x WIDTH de terreno ───────────────────────────

TEST(ServerProtocolMap, RoundTripPreservaMatriz) {
    ServerSerializer serializer;

    // El deserializer exige exactamente HEIGHT x WIDTH celdas.
    std::vector<std::vector<elements>> map(
        HEIGHT, std::vector<elements>(WIDTH, elements::empty));
    map[0][0] = elements::buildings;
    map[HEIGHT - 1][WIDTH - 1] = elements::npcs;

    GameMsg msg(MSG_SEND_MAP);
    msg.set_map(map);

    GameMsg out = round_trip(serializer.serialize_cmd(msg));
    ASSERT_EQ(out.get_map().size(), static_cast<size_t>(HEIGHT));
    ASSERT_EQ(out.get_map()[0].size(), static_cast<size_t>(WIDTH));
    EXPECT_EQ(out.get_map()[0][0], elements::buildings);
    EXPECT_EQ(out.get_map()[HEIGHT - 1][WIDTH - 1], elements::npcs);
    EXPECT_EQ(out.get_map()[5][5], elements::empty);
}
