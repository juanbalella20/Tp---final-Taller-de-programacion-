#ifndef CLIENT_AUDIO_SOUND_PLAYER_H
#define CLIENT_AUDIO_SOUND_PLAYER_H

#include <SDL3_mixer/SDL_mixer.h>

#include <array>
#include <string>
#include <unordered_map>

// Reproductor de efectos de sonido (one-shot): golpes de arma, hechizos, etc.
// A diferencia de ZoneMusicPlayer (música de fondo en loop con crossfade), cada
// sonido se dispara una sola vez y varios pueden solaparse. Usa su propio mixer
// para no interferir con la música; SDL_INIT_AUDIO es refcounted, así que
// inicializarlo dos veces es seguro.
class SoundPlayer {
private:
    static constexpr std::size_t NUM_TRACKS = 8;

    MIX_Mixer* mixer = nullptr;
    std::array<MIX_Track*, NUM_TRACKS> tracks{};
    std::unordered_map<std::string, MIX_Audio*> audio_cache;
    std::size_t next_track = 0;
    float volume = 0.6f;
    bool sdl_audio_initialized = false;
    bool mixer_initialized = false;

    MIX_Audio* load_audio(const std::string& relative_path);
    void release();

public:
    SoundPlayer();
    ~SoundPlayer();

    SoundPlayer(const SoundPlayer&) = delete;
    SoundPlayer& operator=(const SoundPlayer&) = delete;
    SoundPlayer(SoundPlayer&&) = delete;
    SoundPlayer& operator=(SoundPlayer&&) = delete;

    bool is_available() const;
    // Reproduce el .ogg en relative_path una vez. relative_path es relativo a la
    // raíz de assets (igual que ZoneMusicPlayer), p. ej. "client/audio/sounds/x.ogg".
    bool play(const std::string& relative_path);
    void set_volume(float new_volume);
};

#endif  // CLIENT_AUDIO_SOUND_PLAYER_H
