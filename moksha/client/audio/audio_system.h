#pragma once

/**
 * Audio System.
 * This system manages all audio within the game.
 * It handles playing, pausing, and stopping of sound effects and music.
 */

#include <cstdint>

namespace ME {

// Forward declarations.
class Scene;
class AudioSystem;

// Interface for platform-specific audio implementations.
class IAudioImpl {
   public:
    virtual ~IAudioImpl() = default;

    // Initializes the audio implementation with the given audio system.
    virtual void Init(AudioSystem* audioSystem) = 0;

    // Game update.
    virtual void Update(double deltaTime) = 0;

    // Ends the audio implementation, cleaning up resources if necessary.
    virtual void End() = 0;

    // Loads sound file into memory.
    virtual void LoadAudioFile(uint8_t soundId, const char* filePath, bool bSFX) = 0;

    // Unloads sound file from memory.
    virtual void UnloadAudioFile(uint8_t soundId, bool bSFX) = 0;

    // Unload all sound files from memory.
    virtual void UnloadAllAudioFiles() = 0;

    // Plays music track. Used for background music.
    virtual void PlayMusic(uint8_t musicId, bool bLoop) = 0;

    // Stops the currently playing music track.
    virtual void StopMusic() = 0;

    // Plays a sound effect. One shot.
    virtual void PlayAudio(uint8_t soundId, bool bLoop) = 0;

    // Stops a sound effect.
    virtual void StopAudio(uint8_t soundId, bool bLoop) = 0;

    // Sets the master volume for all audio.
    virtual void SetMasterVolume(float volume) = 0;

    // Gets the master volume for all audio.
    virtual float GetMasterVolume() = 0;

    // Toggles mute for all sounds.
    virtual void ToggleMuteAllSounds(bool bMute) = 0;
};

/**
 * Audio System for managing game audio.
 * Will have access to the current scene to manage audio sources tied to scene objects.
 * Will have platform-specific implementations for audio playback.
 */
class AudioSystem {
   public:
    // Initializes the audio system.
    void Init();

    // Updates the audio system with the given delta time.
    void Update(double deltaTime);

    // Ends the audio system, cleaning up resources if necessary.
    void End();

    // Set Scene for the audio system.
    void SetScene(Scene* scene);

    // Plays music track. Used for background music.
    void PlayMusic(uint8_t musicId, bool bLoop);

    // Stops the currently playing music track.
    void StopMusic();

    // Plays a sound effect. One shot.
    void PlayAudio(uint8_t soundId, bool bLoop);

    // Stops a sound effect.
    void StopAudio(uint8_t soundId, bool bLoop);

    // Sets the master volume for all audio.
    void SetMasterVolume(float volume);

    // Gets the master volume for all audio.
    float GetMasterVolume();

    // Toggles mute for all sounds.
    void ToggleMuteAllSounds(bool bMute);

   private:
    bool isInitialized = false;       // Flag to check if the system is initialized.
    ME::Scene* scene = nullptr;       // Current scene being managed.
    IAudioImpl* audioImpl = nullptr;  // Platform-specific audio implementation.
};

}  // namespace ME
