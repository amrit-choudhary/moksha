#pragma once

/**
 * MiniAudio Audio Implementation.
 * This class implements the IAudioImpl interface using the MiniAudio library.
 */

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

    // Loads sound file into memory.
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

    // Toggles mute for all sounds.
    void ToggleMuteAllSounds(bool bMute) override;

   private:
    float masterVolume = 1.0f;  // Master volume level.
    bool isMuted = false;       // Mute state.

    size_t loadedSFXCount = 0;    // Number of loaded sound effects.
    size_t loadedMusicCount = 0;  // Number of loaded music tracks.

    // PIMPL implementation.
    struct Impl;
    Impl* pimpl = nullptr;
};

}  // namespace ME
