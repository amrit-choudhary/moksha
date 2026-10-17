#pragma once

/**
 * Contains data for a sprite animation clip.
 */

#include <cstdint>

namespace ME {

/**
 * Sprite Animation Clip class.
 * This class holds data for a sprite animation clip, including the texture atlas index,
 * the number of sprites, and the indices of the sprites in the atlas.
 */
class SpriteAnimClip {
   public:
    /**
     * Creates a SpriteAnimClip instance.
     * @param bLooping Whether the animation clip should loop.
     * @param textureIndex Index of the texture atlas this animation clip uses.
     * @param spriteCount Number of sprites in this animation clip.
     * @param spriteIndices Array of atlas indices of the sprites in this animation clip.
     * Note: The spriteIndices array not copied. Do not delete it after passing to this constructor.
     */
    SpriteAnimClip(bool bLooping, uint8_t textureIndex, uint8_t spriteCount, const uint16_t* spriteIndices);

    SpriteAnimClip() = delete;
    SpriteAnimClip(const SpriteAnimClip&) = delete;
    SpriteAnimClip& operator=(const SpriteAnimClip&) = delete;
    SpriteAnimClip(SpriteAnimClip&&) = delete;
    SpriteAnimClip& operator=(SpriteAnimClip&&) = delete;

    virtual ~SpriteAnimClip();

    // Get the number of sprites in this animation clip.
    uint8_t GetSpriteCount() const;

    // Check if the animation clip is set to loop.
    bool IsLooping() const;

    // Get the atlasIndex of the sprite at the given index.
    // Checks bounds and return first sprite if out of bounds.
    uint16_t GetSpriteAtlasIndex(uint8_t index) const;

    /**
     * Creates a new SpriteAnimClip instance with all sprite indices offset by the given value.
     * Useful when common sprite animation used for multiple characters with different but similar atlas indices.
     * Helps in reusing the same animation clip data.
     * @param offset The offset to add to each sprite index.
     * Note: delete the original afterwards if not needed.
     */
    static SpriteAnimClip* DuplicateWithOffset(SpriteAnimClip* sourceClip, int16_t offset);

   private:
    // Whether the animation clip should loop.
    bool bLooping = true;
    // Index of the texture atlas this animation clip uses.
    uint8_t textureIndex = 0;
    // Count of sprites in this animation clip.
    const uint8_t spriteCount = 0;
    // Atlas indices of the sprites in this animation clip.
    const uint16_t* spriteIndices = nullptr;
};

}  // namespace ME
