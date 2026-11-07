#pragma once

/**
 * MiniAudio Audio Implementation.
 * This class implements the IAudioImpl interface using the MiniAudio library.
 */

#include <cstddef>
#include <cstdint>

#include "audio_system.h"

namespace ME {

class AudioImplMiniaudio : public IAudioImpl {
   public:
    AudioImplMiniaudio();
    ~AudioImplMiniaudio() override;

    // Initializes the audio implementation with the given audio system.
    void Init(AudioSystem* audioSystem) override;

    // Game update.
    void Update(double deltaTime) override;

    // Ends the audio implementation, cleaning up resources if necessary.
    void End() override;

    // Loads a sound file into memory. filePath is relative to the resources root (read through Vfs).
    void LoadAudioFile(uint8_t soundId, const char* filePath, bool bSFX) override;

    // Unloads sound file from memory.
    void UnloadAudioFile(uint8_t soundId, bool bSFX) override;

    // Unload all sound files from memory.
    void UnloadAllAudioFiles() override;

    // Plays music track. Used for background music.
    void PlayMusic(uint8_t musicId, bool bLoop) override;

    // Stops the currently playing music track.
    void StopMusic() override;

    // Plays a sound effect. One shot.
    void PlayAudio(uint8_t soundId, bool bLoop) override;

    // Stops a sound effect.
    void StopAudio(uint8_t soundId, bool bLoop) override;

    // Sets the master volume for all audio.
    void SetMasterVolume(float volume) override;

    // Gets the master volume for all audio.
    float GetMasterVolume() override;

    // Volume of all music, and of all sound effects, under the master volume (1 = full).
    void SetMusicVolume(float volume) override;
    void SetSfxVolume(float volume) override;

    // Toggles mute for all sounds.
    void ToggleMuteAllSounds(bool bMute) override;

   private:
    float masterVolume = 1.0f;  // Master volume level, kept while muted.
    bool isMuted = false;       // Mute state.

    // PIMPL implementation.
    class Impl;
    Impl* pimpl = nullptr;
};

}  // namespace ME
