#include "zone_music_player.h"

#include <SDL3/SDL.h>

#include <algorithm>
#include <iostream>

#include "paths.h"

ZoneMusicPlayer::ZoneMusicPlayer() {
    if (!SDL_InitSubSystem(SDL_INIT_AUDIO)) {
        std::cerr << "[audio] SDL_InitSubSystem failed: " << SDL_GetError()
                  << std::endl;
        return;
    }
    sdl_audio_initialized = true;

    if (!MIX_Init()) {
        std::cerr << "[audio] MIX_Init failed: " << SDL_GetError() << std::endl;
        release();
        return;
    }
    mixer_initialized = true;

    mixer = MIX_CreateMixerDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, nullptr);
    if (mixer == nullptr) {
        std::cerr << "[audio] MIX_CreateMixerDevice failed: " << SDL_GetError()
                  << std::endl;
        release();
        return;
    }

    for (MIX_Track*& track : tracks) {
        track = MIX_CreateTrack(mixer);
        if (track == nullptr) {
            std::cerr << "[audio] MIX_CreateTrack failed: " << SDL_GetError()
                      << std::endl;
            release();
            return;
        }
    }
}

ZoneMusicPlayer::~ZoneMusicPlayer() {
    release();
}

bool ZoneMusicPlayer::is_available() const {
    return mixer != nullptr && tracks[0] != nullptr && tracks[1] != nullptr;
}

bool ZoneMusicPlayer::play_zone(Zone zone) {
    if (!is_available()) return false;

    const char* relative_path = music_path(zone);
    if (relative_path == nullptr) {
        stop();
        return true;
    }

    MIX_Track* current_track = tracks[active_track];
    if (current_zone == zone && MIX_TrackPlaying(current_track)) {
        return true;
    }

    MIX_Audio* audio = load_audio(relative_path);
    if (audio == nullptr) return false;

    const bool current_is_playing = MIX_TrackPlaying(current_track);
    const std::size_t next_track_index =
        current_is_playing ? 1 - active_track : active_track;
    MIX_Track* next_track = tracks[next_track_index];

    if (MIX_TrackPlaying(next_track)) {
        MIX_StopTrack(next_track, 0);
    }
    if (!MIX_SetTrackAudio(next_track, audio) ||
        !MIX_SetTrackGain(next_track, volume)) {
        std::cerr << "[audio] No se pudo preparar la musica de zona: "
                  << SDL_GetError() << std::endl;
        return false;
    }

    SDL_PropertiesID options = SDL_CreateProperties();
    if (options == 0) {
        std::cerr << "[audio] No se pudieron crear opciones de reproduccion: "
                  << SDL_GetError() << std::endl;
        return false;
    }
    SDL_SetNumberProperty(options, MIX_PROP_PLAY_LOOPS_NUMBER, -1);
    SDL_SetNumberProperty(
        options, MIX_PROP_PLAY_FADE_IN_MILLISECONDS_NUMBER, FADE_MS);

    const bool started = MIX_PlayTrack(next_track, options);
    SDL_DestroyProperties(options);
    if (!started) {
        std::cerr << "[audio] No se pudo reproducir " << relative_path << ": "
                  << SDL_GetError() << std::endl;
        return false;
    }

    if (current_is_playing && next_track_index != active_track) {
        fade_out(current_track);
    }

    active_track = next_track_index;
    current_zone = zone;
    return true;
}

void ZoneMusicPlayer::set_volume(float new_volume) {
    volume = std::clamp(new_volume, 0.0f, 1.0f);
    for (MIX_Track* track : tracks) {
        if (track != nullptr && !MIX_SetTrackGain(track, volume)) {
            std::cerr << "[audio] No se pudo ajustar el volumen: "
                      << SDL_GetError() << std::endl;
        }
    }
}

void ZoneMusicPlayer::stop() {
    if (!is_available()) return;

    for (MIX_Track* track : tracks) {
        if (MIX_TrackPlaying(track)) {
            fade_out(track);
        }
    }
    current_zone.reset();
}

const char* ZoneMusicPlayer::music_path(Zone zone) {
    switch (zone) {
        case ZONE_CITY:
            return "client/audio/music/city.ogg";
        case ZONE_DESERT:
            return "client/audio/music/desert.ogg";
        case ZONE_FOREST:
            return "client/audio/music/nature_music.ogg";
        case ZONE_DUNGEON:
            return "client/audio/music/dungeon.ogg";
        default:
            return nullptr;
    }
}

MIX_Audio* ZoneMusicPlayer::load_audio(const std::string& relative_path) {
    const std::string full_path = paths::asset(relative_path);
    auto cached = audio_cache.find(full_path);
    if (cached != audio_cache.end()) return cached->second;

    // predecode=false mantiene el OGG comprimido y lo decodifica al reproducir.
    MIX_Audio* audio = MIX_LoadAudio(mixer, full_path.c_str(), false);
    if (audio == nullptr) {
        std::cerr << "[audio] No se pudo cargar " << full_path << ": "
                  << SDL_GetError() << std::endl;
        return nullptr;
    }
    audio_cache.emplace(full_path, audio);
    return audio;
}

void ZoneMusicPlayer::fade_out(MIX_Track* track) {
    Sint64 fade_frames = MIX_TrackMSToFrames(track, FADE_MS);
    if (fade_frames < 0) fade_frames = 0;
    if (!MIX_StopTrack(track, fade_frames)) {
        std::cerr << "[audio] No se pudo detener una pista: "
                  << SDL_GetError() << std::endl;
    }
}

void ZoneMusicPlayer::release() {
    for (MIX_Track* track : tracks) {
        if (track != nullptr) {
            MIX_StopTrack(track, 0);
            MIX_SetTrackAudio(track, nullptr);
        }
    }

    for (MIX_Track*& track : tracks) {
        MIX_DestroyTrack(track);
        track = nullptr;
    }

    for (auto& [path, audio] : audio_cache) {
        (void)path;
        MIX_DestroyAudio(audio);
    }
    audio_cache.clear();
    current_zone.reset();
    active_track = 0;

    MIX_DestroyMixer(mixer);
    mixer = nullptr;

    if (mixer_initialized) {
        MIX_Quit();
        mixer_initialized = false;
    }
    if (sdl_audio_initialized) {
        SDL_QuitSubSystem(SDL_INIT_AUDIO);
        sdl_audio_initialized = false;
    }
}
