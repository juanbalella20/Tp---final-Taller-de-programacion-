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
    // Pista reservada para sonidos sostenidos que se cortan a voluntad (p.ej. el
    // sonido mientras gira el anillo de la poción). Separada de las one-shot para
    // poder pararla sin afectar a las demás.
    MIX_Track* hold_track = nullptr;
    std::unordered_map<std::string, MIX_Audio*> audio_cache;
    std::size_t next_track = 0;
    float volume = 0.2f;
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
    // Reproduce el sonido en la pista reservada (one-shot, sin loop). Pensado para
    // sonidos que se cortan antes de terminar con stop_hold(). Si ya había uno
    // sonando en esa pista, lo reemplaza.
    bool play_hold(const std::string& relative_path);
    // Corta el sonido de la pista reservada (no-op si no había nada sonando).
    void stop_hold();
    void set_volume(float new_volume);
};

#endif  // CLIENT_AUDIO_SOUND_PLAYER_H
