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

// Reads relPath through Vfs and creates a sound on engine from the bytes, in group (null: straight to the engine);
// null on failure.
LoadedSound* LoadSound(ma_engine* engine, ma_sound_group* group, const char* relPath) {
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
    if (ma_sound_init_from_data_source(engine, &loaded->decoder, 0, group, &loaded->sound) != MA_SUCCESS) {
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

// Unloads the sound in slot, if any, and empties the slot.
void FreeSlot(LoadedSound*& slot) {
    if (slot != nullptr) {
        UnloadSound(slot);
        slot = nullptr;
    }
}

}  // namespace

// The hidden implementation. Music and effects each play through their own group, under the engine's master volume.
class AudioImplMiniaudio::Impl {
   public:
    ma_context context;
    ma_engine engine;
    ma_sound_group musicGroup;
    ma_sound_group sfxGroup;
    bool bGroups = false;  // both groups exist; without them sounds go straight to the engine

    LoadedSound** sfxs = nullptr;   // MaxLoadedSFXCount slots, indexed by sound id
    LoadedSound** music = nullptr;  // MaxLoadedMusicCount slots, indexed by music id
    bool initialized = false;
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

    if (ma_sound_group_init(&pimpl->engine, 0, NULL, &pimpl->musicGroup) == MA_SUCCESS) {
        if (ma_sound_group_init(&pimpl->engine, 0, NULL, &pimpl->sfxGroup) == MA_SUCCESS) {
            pimpl->bGroups = true;
        } else {
            ma_sound_group_uninit(&pimpl->musicGroup);
        }
    }
    if (!pimpl->bGroups) {
        ME::LogWarning("Audio: couldn't create the music and sfx groups; their volumes won't apply.");
    }

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

        // Sounds first, then their groups, then the engine they all belong to.
        if (pimpl->bGroups) {
            ma_sound_group_uninit(&pimpl->sfxGroup);
            ma_sound_group_uninit(&pimpl->musicGroup);
            pimpl->bGroups = false;
        }
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
        // Loading over an id replaces its sound.
        FreeSlot(pimpl->sfxs[soundId]);
        pimpl->sfxs[soundId] = LoadSound(&pimpl->engine, pimpl->bGroups ? &pimpl->sfxGroup : nullptr, filePath);
        if (pimpl->sfxs[soundId] == nullptr) {
            ME::LogError("Failed to load SFX file:", filePath);
        }
    } else {
        if (soundId >= ME::Constants::MaxLoadedMusicCount) {
            ME::LogError("Music id out of range:", soundId);
            return;
        }
        FreeSlot(pimpl->music[soundId]);
        pimpl->music[soundId] = LoadSound(&pimpl->engine, pimpl->bGroups ? &pimpl->musicGroup : nullptr, filePath);
        if (pimpl->music[soundId] == nullptr) {
            ME::LogError("Failed to load music file:", filePath);
        }
    }
}

void AudioImplMiniaudio::UnloadAudioFile(uint8_t soundId, bool bSFX) {
    if (pimpl == nullptr) {
        return;
    }
    if (bSFX && soundId < ME::Constants::MaxLoadedSFXCount) {
        FreeSlot(pimpl->sfxs[soundId]);
    } else if (!bSFX && soundId < ME::Constants::MaxLoadedMusicCount) {
        FreeSlot(pimpl->music[soundId]);
    }
}

void AudioImplMiniaudio::UnloadAllAudioFiles() {
    if (pimpl == nullptr) {
        return;
    }

    // Every slot, since ids needn't be loaded in order.
    for (size_t i = 0; i < ME::Constants::MaxLoadedSFXCount; ++i) {
        FreeSlot(pimpl->sfxs[i]);
    }
    for (size_t i = 0; i < ME::Constants::MaxLoadedMusicCount; ++i) {
        FreeSlot(pimpl->music[i]);
    }
}

void AudioImplMiniaudio::PlayMusic(uint8_t musicId, bool bLoop) {
    if (pimpl == nullptr || musicId >= ME::Constants::MaxLoadedMusicCount || pimpl->music[musicId] == nullptr) {
        return;
    }

    // One track at a time: a new track replaces the one playing.
    StopMusic();
    ma_sound* music = &pimpl->music[musicId]->sound;
    ma_sound_set_looping(music, bLoop ? MA_TRUE : MA_FALSE);
    // Start playing the music from the beginning.
    ma_sound_seek_to_pcm_frame(music, 0);
    ma_sound_start(music);
}

void AudioImplMiniaudio::StopMusic() {
    if (pimpl == nullptr) {
        return;
    }
    for (size_t i = 0; i < ME::Constants::MaxLoadedMusicCount; ++i) {
        if (pimpl->music[i] != nullptr) {
            ma_sound_stop(&pimpl->music[i]->sound);
        }
    }
}

void AudioImplMiniaudio::PlayAudio(uint8_t soundId, bool bLoop) {
    if (pimpl == nullptr || soundId >= ME::Constants::MaxLoadedSFXCount || pimpl->sfxs[soundId] == nullptr) {
        return;
    }

    ma_sound* sound = &pimpl->sfxs[soundId]->sound;
    ma_sound_set_looping(sound, bLoop ? MA_TRUE : MA_FALSE);
    // Start playing the sound from the beginning.
    ma_sound_seek_to_pcm_frame(sound, 0);
    ma_sound_start(sound);
}

void AudioImplMiniaudio::StopAudio(uint8_t soundId, bool bLoop) {
    if (pimpl == nullptr || soundId >= ME::Constants::MaxLoadedSFXCount || pimpl->sfxs[soundId] == nullptr) {
        return;
    }
    ma_sound_stop(&pimpl->sfxs[soundId]->sound);
}

void AudioImplMiniaudio::SetMasterVolume(float volume) {
    masterVolume = volume;
    // While muted the new volume is kept and applies on unmute.
    if (pimpl != nullptr && !isMuted) {
        ma_engine_set_volume(&pimpl->engine, masterVolume);
    }
}

float AudioImplMiniaudio::GetMasterVolume() {
    return masterVolume;
}

void AudioImplMiniaudio::SetMusicVolume(float volume) {
    if (pimpl != nullptr && pimpl->bGroups) {
        ma_sound_group_set_volume(&pimpl->musicGroup, volume);
    }
}

void AudioImplMiniaudio::SetSfxVolume(float volume) {
    if (pimpl != nullptr && pimpl->bGroups) {
        ma_sound_group_set_volume(&pimpl->sfxGroup, volume);
    }
}

void AudioImplMiniaudio::ToggleMuteAllSounds(bool bMute) {
    isMuted = bMute;
    if (pimpl != nullptr) {
        ma_engine_set_volume(&pimpl->engine, isMuted ? 0.0f : masterVolume);
    }
}

}  // namespace ME
