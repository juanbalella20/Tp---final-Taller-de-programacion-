#include "sound_player.h"

#include <SDL3/SDL.h>

#include <algorithm>
#include <iostream>

#include "paths.h"

SoundPlayer::SoundPlayer() {
    if (!SDL_InitSubSystem(SDL_INIT_AUDIO)) {
        std::cerr << "[sfx] SDL_InitSubSystem failed: " << SDL_GetError()
                  << std::endl;
        return;
    }
    sdl_audio_initialized = true;

    if (!MIX_Init()) {
        std::cerr << "[sfx] MIX_Init failed: " << SDL_GetError() << std::endl;
        release();
        return;
    }
    mixer_initialized = true;

    mixer = MIX_CreateMixerDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, nullptr);
    if (mixer == nullptr) {
        std::cerr << "[sfx] MIX_CreateMixerDevice failed: " << SDL_GetError()
                  << std::endl;
        release();
        return;
    }

    for (MIX_Track*& track : tracks) {
        track = MIX_CreateTrack(mixer);
        if (track == nullptr) {
            std::cerr << "[sfx] MIX_CreateTrack failed: " << SDL_GetError()
                      << std::endl;
            release();
            return;
        }
    }

    hold_track = MIX_CreateTrack(mixer);
    if (hold_track == nullptr) {
        std::cerr << "[sfx] MIX_CreateTrack (hold) failed: " << SDL_GetError()
                  << std::endl;
        release();
        return;
    }
}

SoundPlayer::~SoundPlayer() {
    release();
}

bool SoundPlayer::is_available() const {
    return mixer != nullptr && tracks[0] != nullptr;
}

bool SoundPlayer::play(const std::string& relative_path) {
    if (!is_available()) return false;

    MIX_Audio* audio = load_audio(relative_path);
    if (audio == nullptr) return false;

    // Round-robin entre las pistas para que varios efectos puedan solaparse en
    // vez de cortarse entre sí.
    MIX_Track* track = tracks[next_track];
    next_track = (next_track + 1) % NUM_TRACKS;

    if (MIX_TrackPlaying(track)) {
        MIX_StopTrack(track, 0);
    }
    if (!MIX_SetTrackAudio(track, audio) || !MIX_SetTrackGain(track, volume)) {
        std::cerr << "[sfx] No se pudo preparar el sonido: " << SDL_GetError()
                  << std::endl;
        return false;
    }

    if (!MIX_PlayTrack(track, 0)) {
        std::cerr << "[sfx] No se pudo reproducir " << relative_path << ": "
                  << SDL_GetError() << std::endl;
        return false;
    }
    return true;
}

bool SoundPlayer::play_hold(const std::string& relative_path) {
    if (!is_available() || hold_track == nullptr) return false;

    MIX_Audio* audio = load_audio(relative_path);
    if (audio == nullptr) return false;

    if (MIX_TrackPlaying(hold_track)) {
        MIX_StopTrack(hold_track, 0);
    }
    if (!MIX_SetTrackAudio(hold_track, audio) ||
        !MIX_SetTrackGain(hold_track, volume)) {
        std::cerr << "[sfx] No se pudo preparar el sonido sostenido: "
                  << SDL_GetError() << std::endl;
        return false;
    }
    if (!MIX_PlayTrack(hold_track, 0)) {
        std::cerr << "[sfx] No se pudo reproducir " << relative_path << ": "
                  << SDL_GetError() << std::endl;
        return false;
    }
    return true;
}

void SoundPlayer::stop_hold() {
    if (hold_track != nullptr && MIX_TrackPlaying(hold_track)) {
        MIX_StopTrack(hold_track, 0);
    }
}

void SoundPlayer::set_volume(float new_volume) {
    volume = std::clamp(new_volume, 0.0f, 1.0f);
}

MIX_Audio* SoundPlayer::load_audio(const std::string& relative_path) {
    const std::string full_path = paths::asset(relative_path);
    auto cached = audio_cache.find(full_path);
    if (cached != audio_cache.end()) return cached->second;

    // predecode=true: los efectos son cortos y se disparan a menudo, conviene
    // tenerlos decodificados en memoria para que el play no tenga latencia.
    MIX_Audio* audio = MIX_LoadAudio(mixer, full_path.c_str(), true);
    if (audio == nullptr) {
        std::cerr << "[sfx] No se pudo cargar " << full_path << ": "
                  << SDL_GetError() << std::endl;
        return nullptr;
    }
    audio_cache.emplace(full_path, audio);
    return audio;
}

void SoundPlayer::release() {
    for (MIX_Track*& track : tracks) {
        if (track != nullptr) {
            MIX_StopTrack(track, 0);
            MIX_SetTrackAudio(track, nullptr);
            MIX_DestroyTrack(track);
            track = nullptr;
        }
    }

    if (hold_track != nullptr) {
        MIX_StopTrack(hold_track, 0);
        MIX_SetTrackAudio(hold_track, nullptr);
        MIX_DestroyTrack(hold_track);
        hold_track = nullptr;
    }

    for (auto& [path, audio] : audio_cache) {
        (void)path;
        MIX_DestroyAudio(audio);
    }
    audio_cache.clear();
    next_track = 0;

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
