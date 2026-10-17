#include "sprite_animator.h"

#include "moksha/client/rendering/shared/sprite_renderer.h"
#include "moksha/logging/logging.h"
#include "moksha/shared/misc/engine_limits.h"

ME::SpriteAnimator::SpriteAnimator(ME::SpriteRenderer* spriteRenderer, uint8_t frameRate)
    : spriteRenderer(spriteRenderer) {
    if (spriteRenderer == nullptr) {
        ME::LogError("SpriteAnimator created without a SpriteRenderer!");
    }
    clips = new ME::SpriteAnimClip* [ME::Constants::MaxSpriteAnimClipsOnAnimator] {};
    SetFrameRate(frameRate);
}

ME::SpriteAnimator::~SpriteAnimator() {
    for (uint8_t i = 0; i < clipCount; ++i) {
        if (clips[i] != nullptr) {
            delete clips[i];
        }
    }
    delete[] clips;
}

void ME::SpriteAnimator::AddClip(ME::SpriteAnimClip* clip) {
    if (clipCount < ME::Constants::MaxSpriteAnimClipsOnAnimator) {
        clips[clipCount] = clip;
        ++clipCount;
    } else {
        ME::LogError("Max sprite animation clips reached on animator!");
    }
}

void ME::SpriteAnimator::ChangeClip(uint8_t clipIndex) {
    if (clipIndex < clipCount) {
        currentClipIndex = clipIndex;
        currentFrame = 0;
        accumulatedFrames = 0;

        UpdateSpriteRenderer();
    } else {
        ME::LogError("Sprite animation clip index out of bounds!");
    }
}

void ME::SpriteAnimator::AnimationUpdate() {
    if (!bRunning) {
        return;
    }
    if (clipCount == 0) {
        return;
    }
    accumulatedFrames++;
    if (accumulatedFrames >= skipFrameCount) {
        accumulatedFrames = 0;
        ++currentFrame;
        ME::SpriteAnimClip* currentClip = clips[currentClipIndex];
        if (currentFrame >= currentClip->GetSpriteCount()) {
            if (currentClip->IsLooping()) {
                currentFrame = 0;
            } else {
                currentFrame = currentClip->GetSpriteCount() - 1;
            }
        }

        UpdateSpriteRenderer();
    }
}

void ME::SpriteAnimator::SetFrameRate(uint8_t frameRate) {
    // avoid 0 fps because of division by zero.
    if (frameRate == 0) {
        frameRate = DEFAULT_FRAME_RATE;
    }

    this->frameRate = frameRate;
    skipFrameCount = ME::Constants::AnimGameFrameRate / frameRate;
}

uint16_t ME::SpriteAnimator::GetCurrentAtlasIndex() const {
    if (clipCount == 0) {
        return 0;
    }
    return clips[currentClipIndex]->GetSpriteAtlasIndex(currentFrame);
}

uint8_t ME::SpriteAnimator::GetCurrentClipIndex() const {
    return currentClipIndex;
}

bool ME::SpriteAnimator::IsAnimationRunning() const {
    return bRunning;
}

void ME::SpriteAnimator::StartAnimation() {
    bRunning = true;
}

void ME::SpriteAnimator::StopAnimation() {
    bRunning = false;
}

void ME::SpriteAnimator::UpdateSpriteRenderer() {
    if (clipCount == 0) {
        return;
    }
    spriteRenderer->atlasIndex = clips[currentClipIndex]->GetSpriteAtlasIndex(currentFrame);
    spriteRenderer->bDirty = true;
}
