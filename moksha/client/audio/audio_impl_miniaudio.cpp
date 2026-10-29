#include "audio_impl_miniaudio.h"

#include "moksha/client/third_party/miniaudio-0.11.23/miniaudio.h"
#include "moksha/logging/logging.h"
#include "moksha/shared/file_io/vfs.h"
#include "moksha/shared/misc/engine_limits.h"

namespace ME {

// One sound decoded from its file bytes, which stay alive as long as the sound does.
class LoadedSound {
   public:
    ma_sound sound;
    ma_decoder decoder;
    uint8_t* bytes = nullptr;
};

namespace {

// Reads relPath through Vfs and creates a sound on engine from the bytes; null on failure.
LoadedSound* LoadSound(ma_engine* engine, const char* relPath) {
    size_t size = 0;
    if (!Vfs::GetFileSize(FileRoot::Resources, relPath, size) || size == 0) {
        return nullptr;
    }
    LoadedSound* loaded = new LoadedSound();
    loaded->bytes = new uint8_t[size];
    if (!Vfs::ReadBytes(FileRoot::Resources, relPath, loaded->bytes, size, size) ||
        ma_decoder_init_memory(loaded->bytes, size, NULL, &loaded->decoder) != MA_SUCCESS) {
        delete[] loaded->bytes;
        delete loaded;
        return nullptr;
    }
    if (ma_sound_init_from_data_source(engine, &loaded->decoder, 0, NULL, &loaded->sound) != MA_SUCCESS) {
        ma_decoder_uninit(&loaded->decoder);
        delete[] loaded->bytes;
        delete loaded;
        return nullptr;
    }
    return loaded;
}

void UnloadSound(LoadedSound* loaded) {
    ma_sound_uninit(&loaded->sound);
    ma_decoder_uninit(&loaded->decoder);
    delete[] loaded->bytes;
    delete loaded;
}

}  // namespace

// Define the hidden implementation
struct AudioImplMiniaudio::Impl {
    ma_context context;
    ma_engine engine;

    // Array of loaded sound effects.
    LoadedSound** sfxs;

    // Array of loaded music tracks.
    LoadedSound** music;
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

    // An explicit context sets the iOS audio session: Ambient respects the silent switch and mixes with other apps'
    // audio (miniaudio's default records). Other platforms ignore the coreaudio session fields.
    ma_context_config contextConfig = ma_context_config_init();
    contextConfig.coreaudio.sessionCategory = ma_ios_session_category_ambient;
    if (ma_context_init(NULL, 0, &contextConfig, &pimpl->context) != MA_SUCCESS) {
        delete pimpl;
        pimpl = nullptr;
        return;
    }

    ma_engine_config engineConfig = ma_engine_config_init();
    engineConfig.pContext = &pimpl->context;
    if (ma_engine_init(&engineConfig, &pimpl->engine) != MA_SUCCESS) {
        ma_context_uninit(&pimpl->context);
        delete pimpl;
        pimpl = nullptr;
        return;
    }
    pimpl->initialized = true;

    // Allocate arrays for sound effects and music tracks.
    pimpl->sfxs = new LoadedSound* [ME::Constants::MaxLoadedSFXCount] {};
    pimpl->music = new LoadedSound* [ME::Constants::MaxLoadedMusicCount] {};
}

void AudioImplMiniaudio::Update(double deltaTime) {}

void AudioImplMiniaudio::End() {
    if (pimpl == nullptr) {
        return;
    }

    if (pimpl->initialized) {
        UnloadAllAudioFiles();

        ma_engine_uninit(&pimpl->engine);
        ma_context_uninit(&pimpl->context);
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
        if (soundId >= ME::Constants::MaxLoadedSFXCount) {
            ME::LogError("SFX id out of range:", soundId);
            return;
        }
        pimpl->sfxs[soundId] = LoadSound(&pimpl->engine, filePath);
        ++loadedSFXCount;

        if (pimpl->sfxs[soundId] == nullptr) {
            ME::LogError("Failed to load SFX file:", filePath);
        }
    } else {
        if (soundId >= ME::Constants::MaxLoadedMusicCount) {
            ME::LogError("Music id out of range:", soundId);
            return;
        }
        pimpl->music[soundId] = LoadSound(&pimpl->engine, filePath);
        ++loadedMusicCount;

        if (pimpl->music[soundId] == nullptr) {
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
            UnloadSound(pimpl->sfxs[i]);
            pimpl->sfxs[i] = nullptr;
        }
    }
    loadedSFXCount = 0;

    // Unload all music tracks.
    for (size_t i = 0; i < loadedMusicCount; ++i) {
        if (pimpl->music[i] != nullptr) {
            UnloadSound(pimpl->music[i]);
            pimpl->music[i] = nullptr;
        }
    }
    loadedMusicCount = 0;
}

void AudioImplMiniaudio::PlayMusic(uint8_t musicId, bool bLoop) {
    if (pimpl == nullptr) {
        return;
    }

    ma_sound* music = pimpl->music[musicId] != nullptr ? &pimpl->music[musicId]->sound : nullptr;
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

    ma_sound* sound = pimpl->sfxs[soundId] != nullptr ? &pimpl->sfxs[soundId]->sound : nullptr;
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
