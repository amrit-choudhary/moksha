#include "sprite_anim_clip.h"

ME::SpriteAnimClip::SpriteAnimClip(bool bLooping, uint8_t textureIndex, const uint8_t spriteCount,
                                   const uint16_t* spriteIndices)
    : bLooping(bLooping), textureIndex(textureIndex), spriteCount(spriteCount), spriteIndices(spriteIndices) {
    // vars are initialized in initializer list because some are constant.
}

ME::SpriteAnimClip::~SpriteAnimClip() {
    delete[] spriteIndices;
    spriteIndices = nullptr;
}

uint8_t ME::SpriteAnimClip::GetSpriteCount() const {
    return spriteCount;
}

bool ME::SpriteAnimClip::IsLooping() const {
    return bLooping;
}

uint16_t ME::SpriteAnimClip::GetSpriteAtlasIndex(uint8_t index) const {
    if (spriteCount == 0) {
        return 0;
    }
    if (index < spriteCount) {
        return spriteIndices[index];
    }
    return spriteIndices[0];
}

ME::SpriteAnimClip* ME::SpriteAnimClip::DuplicateWithOffset(ME::SpriteAnimClip* sourceClip, int16_t offset) {
    if (sourceClip == nullptr) {
        return nullptr;
    }

    uint8_t spriteCount = sourceClip->GetSpriteCount();
    const uint16_t* sourceIndices = sourceClip->spriteIndices;

    // Create a new array for the offset indices.
    uint16_t* offsetIndices = new uint16_t[spriteCount];
    for (uint8_t i = 0; i < spriteCount; ++i) {
        offsetIndices[i] = static_cast<uint16_t>(static_cast<int16_t>(sourceIndices[i]) + offset);

        if (offsetIndices[i] < 0) {
            offsetIndices[i] = 0;
        }
    }

    // Create a new SpriteAnimClip with the offset indices.
    SpriteAnimClip* newClip =
        new SpriteAnimClip(sourceClip->bLooping, sourceClip->textureIndex, spriteCount, offsetIndices);

    return newClip;
}
