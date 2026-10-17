#pragma once

#include <cstdint>

#include "sprite_anim_clip.h"

namespace ME {

// Forward declarations.
class SpriteRenderer;

class SpriteAnimator {
   public:
    SpriteAnimator(ME::SpriteRenderer* spriteRenderer, uint8_t frameRate = 24);
    virtual ~SpriteAnimator();

    /**
     * Add an animation clip to the animator.
     * NOTE:
     * Create SpriteAnimClip using 'new' and pass the pointer to this method.
     * Don't store the pointer elsewhere, as the animator will take ownership and delete it.
     * Dont send stack allocated or global/static clips as they can get deleted automatically.
     */
    void AddClip(ME::SpriteAnimClip* clip);

    /**
     * Change the active animation clip by index.
     */
    void ChangeClip(uint8_t clipIndex);

    // Anim system update message. Advance the current frame based on frame rate.
    void AnimationUpdate();

    /**
     * Set the frame rate (frames per second) for the animator.
     * Setting it to 0 is not possible, will set it to defaultFrameRate.
     */
    void SetFrameRate(uint8_t frameRate);

    /**
     * Return the resultant current frame atlasIndex of the active animation clip.
     */
    uint16_t GetCurrentAtlasIndex() const;

    /**
     * Return the current animation clip index.
     */
    uint8_t GetCurrentClipIndex() const;

    /**
     * Check if the animation is currently running.
     */
    bool IsAnimationRunning() const;

    /**
     * Will pause the animation update.
     */
    void StartAnimation();

    /**
     * Will resume the animation update.
     */
    void StopAnimation();

   private:
    ME::SpriteRenderer* spriteRenderer = nullptr;  // Associated sprite renderer.
    bool bRunning = true;
    uint8_t currentFrame = 0;
    uint8_t clipCount = 0;
    uint8_t currentClipIndex = 0;
    ME::SpriteAnimClip** clips = nullptr;
    uint8_t accumulatedFrames = 0;
    uint8_t skipFrameCount = 0;
    uint8_t frameRate = DEFAULT_FRAME_RATE;
    constexpr static uint8_t DEFAULT_FRAME_RATE = 24;

    // Apply the latest frame's atlas index to the sprite renderer after animation update.
    void UpdateSpriteRenderer();
};

}  // namespace ME
