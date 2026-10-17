#include "audio_system.h"

#include "audio_impl_miniaudio.h"
#include "moksha/client/scene/scene.h"
#include "moksha/shared/misc/utils.h"

void ME::AudioSystem::Init() {
    isInitialized = false;
    audioImpl = new AudioImplMiniaudio();
    audioImpl->Init(this);
    isInitialized = true;
}

void ME::AudioSystem::Update(double deltaTime) {
    if (scene == nullptr || !isInitialized) {
        return;
    }

    // Audio system update logic would go here.
    audioImpl->Update(deltaTime);
}

void ME::AudioSystem::End() {
    if (audioImpl) {
        audioImpl->End();
        delete audioImpl;
        audioImpl = nullptr;
    }
    isInitialized = false;
}

void ME::AudioSystem::SetScene(Scene* scene) {
    this->scene = scene;  // Set the current scene.

    // Load audio resources related to the scene here.
    for (uint8_t i = 0; i < scene->sfxPaths.count; ++i) {
        std::string fileName = ME::Utils::GetResourcesPath() + scene->sfxPaths[i];
        audioImpl->LoadAudioFile(i, fileName.c_str(), true);
    }

    for (uint8_t i = 0; i < scene->musicPaths.count; ++i) {
        std::string fileName = ME::Utils::GetResourcesPath() + scene->musicPaths[i];
        audioImpl->LoadAudioFile(i, fileName.c_str(), false);
    }

    isInitialized = true;  // Mark the system as initialized.
}

void ME::AudioSystem::PlayMusic(uint8_t musicId, bool bLoop) {
    audioImpl->PlayMusic(musicId, bLoop);
}

void ME::AudioSystem::StopMusic() {}

void ME::AudioSystem::PlayAudio(uint8_t soundId, bool bLoop) {
    audioImpl->PlayAudio(soundId, bLoop);
}

void ME::AudioSystem::StopAudio(uint8_t soundId, bool bLoop) {}

void ME::AudioSystem::SetMasterVolume(float volume) {
    audioImpl->SetMasterVolume(volume);
}

float ME::AudioSystem::GetMasterVolume() {
    return audioImpl->GetMasterVolume();
}

void ME::AudioSystem::ToggleMuteAllSounds(bool bMute) {}
