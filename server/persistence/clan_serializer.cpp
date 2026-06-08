#include "clan_serializer.h"

#include <cstring>
#include <vector>

void ClanSerializer::copy_to_fixed(char* dst, size_t n, const std::string& src) const {
    std::memset(dst, 0, n);
    std::strncpy(dst, src.c_str(), n - 1);
}

std::string ClanSerializer::read_fixed(const char* src, size_t n) const {
    return std::string(src, ::strnlen(src, n));
}

ClanRecord ClanSerializer::to_record(const Clan& clan) const {
    ClanRecord rec;
    std::memset(&rec, 0, sizeof(rec));

    copy_to_fixed(rec.founder, PERSIST_NAME_MAX, clan.get_founder());
    copy_to_fixed(rec.clan_name, PERSIST_CLAN_NAME_MAX, clan.get_name());
    copy_to_fixed(rec.review, PERSIST_CLAN_REVIEW_MAX, clan.get_review());

    // Miembros y baneados: un slot fijo por nombre, truncando si exceden el cupo.
    uint8_t mc = 0;
    for (const std::string& name : clan.get_members()) {
        if (mc >= PERSIST_CLAN_MEMBERS_MAX) break;
        copy_to_fixed(rec.members[mc], PERSIST_NAME_MAX, name);
        ++mc;
    }
    rec.member_count = mc;

    uint8_t bc = 0;
    for (const std::string& name : clan.get_banned()) {
        if (bc >= PERSIST_CLAN_BANNED_MAX) break;
        copy_to_fixed(rec.banned[bc], PERSIST_NAME_MAX, name);
        ++bc;
    }
    rec.banned_count = bc;

    return rec;
}

Clan ClanSerializer::from_record(const ClanRecord& rec) const {
    std::string founder = read_fixed(rec.founder, PERSIST_NAME_MAX);
    std::string clan_name = read_fixed(rec.clan_name, PERSIST_CLAN_NAME_MAX);
    std::string review = read_fixed(rec.review, PERSIST_CLAN_REVIEW_MAX);

    std::vector<std::string> members;
    for (uint8_t i = 0; i < rec.member_count && i < PERSIST_CLAN_MEMBERS_MAX; ++i) {
        members.push_back(read_fixed(rec.members[i], PERSIST_NAME_MAX));
    }

    std::vector<std::string> banned;
    for (uint8_t i = 0; i < rec.banned_count && i < PERSIST_CLAN_BANNED_MAX; ++i) {
        banned.push_back(read_fixed(rec.banned[i], PERSIST_NAME_MAX));
    }

    return Clan(founder, clan_name, review, std::move(members), std::move(banned));
}
