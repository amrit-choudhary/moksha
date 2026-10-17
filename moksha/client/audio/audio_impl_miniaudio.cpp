#include "audio_impl_miniaudio.h"

#include "moksha/client/third_party/miniaudio-0.11.23/miniaudio.h"
#include "moksha/logging/logging.h"
#include "moksha/shared/misc/engine_limits.h"

namespace ME {

// Define the hidden implementation
struct AudioImplMiniaudio::Impl {
    ma_engine engine;

    // Array of loaded sound effects.
    ma_sound** sfxs;

    // Array of loaded music tracks.
    ma_sound** music;
    bool initialized = false;
    Impl() = default;
    ~Impl() = default;
};

AudioImplMiniaudio::AudioImplMiniaudio() = default;

AudioImplMiniaudio::~AudioImplMiniaudio() {
    End();
}

void AudioImplMiniaudio::Init(AudioSystem* audioSystem) {
    if (pimpl == nullptr) {
        pimpl = new Impl();
    }

    if (ma_engine_init(NULL, &pimpl->engine) != MA_SUCCESS) {
        delete pimpl;
        pimpl = nullptr;
        return;
    }
    pimpl->initialized = true;

    // Allocate arrays for sound effects and music tracks.
    pimpl->sfxs = new ma_sound* [ME::Constants::MaxLoadedSFXCount] {};
    pimpl->music = new ma_sound* [ME::Constants::MaxLoadedMusicCount] {};
}

void AudioImplMiniaudio::Update(double deltaTime) {}

void AudioImplMiniaudio::End() {
    if (pimpl == nullptr) {
        return;
    }

    if (pimpl->initialized) {
        UnloadAllAudioFiles();

        ma_engine_uninit(&pimpl->engine);
        pimpl->initialized = false;
    }

    delete[] pimpl->sfxs;
    delete[] pimpl->music;

    delete pimpl;
    pimpl = nullptr;
}

void AudioImplMiniaudio::LoadAudioFile(uint8_t soundId, const char* filePath, bool bSFX) {
    if (pimpl == nullptr) {
        return;
    }

    if (bSFX) {
        pimpl->sfxs[soundId] = new ma_sound();
        ma_result result =
            ma_sound_init_from_file(&pimpl->engine, filePath, MA_SOUND_FLAG_DECODE, NULL, NULL, pimpl->sfxs[soundId]);
        ++loadedSFXCount;

        if (result != MA_SUCCESS) {
            ME::LogError("Failed to load SFX file:", filePath);
        }
    } else {
        pimpl->music[soundId] = new ma_sound();
        ma_result result =
            ma_sound_init_from_file(&pimpl->engine, filePath, MA_SOUND_FLAG_DECODE, NULL, NULL, pimpl->music[soundId]);
        ++loadedMusicCount;

        if (result != MA_SUCCESS) {
            ME::LogError("Failed to load music file:", filePath);
        }
    }
}

void AudioImplMiniaudio::UnloadAudioFile(uint8_t soundId, bool bSFX) {}

void AudioImplMiniaudio::UnloadAllAudioFiles() {
    if (pimpl == nullptr) {
        return;
    }

    // Unload all sound effects.
    for (size_t i = 0; i < loadedSFXCount; ++i) {
        if (pimpl->sfxs[i] != nullptr) {
            ma_sound_uninit(pimpl->sfxs[i]);
            delete pimpl->sfxs[i];
            pimpl->sfxs[i] = nullptr;
        }
    }
    loadedSFXCount = 0;

    // Unload all music tracks.
    for (size_t i = 0; i < loadedMusicCount; ++i) {
        if (pimpl->music[i] != nullptr) {
            ma_sound_uninit(pimpl->music[i]);
            delete pimpl->music[i];
            pimpl->music[i] = nullptr;
        }
    }
    loadedMusicCount = 0;
}

void AudioImplMiniaudio::PlayMusic(uint8_t musicId, bool bLoop) {
    if (pimpl == nullptr) {
        return;
    }

    ma_sound* music = pimpl->music[musicId];
    if (music != nullptr) {
        ma_sound_set_looping(music, bLoop ? MA_TRUE : MA_FALSE);
        // Start playing the music from the beginning.
        ma_sound_seek_to_pcm_frame(music, 0);
        ma_sound_start(music);
    }
}

void AudioImplMiniaudio::StopMusic() {}

void AudioImplMiniaudio::PlayAudio(uint8_t soundId, bool bLoop) {
    if (pimpl == nullptr) {
        return;
    }

    ma_sound* sound = pimpl->sfxs[soundId];
    if (sound != nullptr) {
        ma_sound_set_looping(sound, bLoop ? MA_TRUE : MA_FALSE);
        // Start playing the sound from the beginning.
        ma_sound_seek_to_pcm_frame(sound, 0);
        ma_sound_start(sound);
    }
}

void AudioImplMiniaudio::StopAudio(uint8_t soundId, bool bLoop) {}

void AudioImplMiniaudio::SetMasterVolume(float volume) {
    masterVolume = volume;
    ma_engine_set_volume(&pimpl->engine, masterVolume);
}

float AudioImplMiniaudio::GetMasterVolume() {
    return masterVolume;
}

void AudioImplMiniaudio::ToggleMuteAllSounds(bool bMute) {
    isMuted = bMute;
}

}  // namespace ME
