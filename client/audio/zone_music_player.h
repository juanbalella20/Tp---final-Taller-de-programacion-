#ifndef CLIENT_AUDIO_ZONE_MUSIC_PLAYER_H
#define CLIENT_AUDIO_ZONE_MUSIC_PLAYER_H

#include <SDL3_mixer/SDL_mixer.h>

#include <array>
#include <optional>
#include <string>
#include <unordered_map>

#include "protocol_constants.h"

class ZoneMusicPlayer {
private:
    MIX_Mixer* mixer = nullptr;
    std::array<MIX_Track*, 2> tracks{nullptr, nullptr};
    std::unordered_map<std::string, MIX_Audio*> audio_cache;
    std::optional<Zone> current_zone;
    std::size_t active_track = 0;
    float volume = 0.05f;
    bool sdl_audio_initialized = false;
    bool mixer_initialized = false;

    static constexpr Sint64 FADE_MS = 500;

    static const char* music_path(Zone zone);
    MIX_Audio* load_audio(const std::string& relative_path);
    void fade_out(MIX_Track* track);
    void release();

public:
    ZoneMusicPlayer();
    ~ZoneMusicPlayer();

    ZoneMusicPlayer(const ZoneMusicPlayer&) = delete;
    ZoneMusicPlayer& operator=(const ZoneMusicPlayer&) = delete;
    ZoneMusicPlayer(ZoneMusicPlayer&&) = delete;
    ZoneMusicPlayer& operator=(ZoneMusicPlayer&&) = delete;

    bool is_available() const;
    bool play_zone(Zone zone);
    void set_volume(float new_volume);
    void stop();
};

#endif  // CLIENT_AUDIO_ZONE_MUSIC_PLAYER_H
