#ifdef ME_VULKAN

#include "scene_vulkan.h"

#include "moksha/client/rendering/shared/shader_constants.h"
#include "moksha/logging/logging.h"

bool ME::SceneVulkan::Init(VkPhysicalDevice physicalDevice, VkDevice inDevice, VkQueue queue, VkCommandPool uploadPool,
                           VkDescriptorSetLayout perPassLayout, VkDescriptorSetLayout perSpriteLayout,
                           VkDescriptorSetLayout instancedLayout, VkDescriptorSetLayout textureLayout,
                           ME::Scene* gameScene) {
    device = inDevice;
    scene = gameScene;
    spriteRenderers = gameScene->spriteRenderers;

    instancedSpriteRenderers0 = scene->instancedSpriteRenderers0;
    spriteInstanceData0 = scene->spriteInstanceData0;

    instancedSpriteRenderers1 = scene->instancedSpriteRenderers1;
    spriteInstanceData1 = scene->spriteInstanceData1;

    if (!MakeSpriteTextures(physicalDevice, queue, uploadPool)) {
        return false;
    }

    if (!MakeBuffers(physicalDevice)) {
        return false;
    }

    if (!MakeSpriteInstanceBuffer(physicalDevice)) {
        return false;
    }

    if (!MakeDescriptorSets(perPassLayout, perSpriteLayout, instancedLayout, textureLayout)) {
        return false;
    }

    ME::Log("SceneVulkan: ", gameScene->spriteTexturePaths.count, " sprite textures, ",
            gameScene->spriteRenderers.count, " sprites, ", gameScene->textureAtlasProperties.count, " atlases, ",
            "instanced ", instanceCount0, " + ", instanceCount1);

    return true;
}

void ME::SceneVulkan::End() {
    if (descriptorPool != VK_NULL_HANDLE) {
        vkDestroyDescriptorPool(device, descriptorPool, nullptr);  // frees every set made from it
        descriptorPool = VK_NULL_HANDLE;
    }
    perPassSet = VK_NULL_HANDLE;
    perSpriteSet = VK_NULL_HANDLE;
    for (uint32_t i = 0; i < Constants::MaxSpriteTextureCount; ++i) {
        textureSets[i] = VK_NULL_HANDLE;
    }
    spriteInstanceSet0 = VK_NULL_HANDLE;
    spriteInstanceSet1 = VK_NULL_HANDLE;

    // Harmless for a set that was empty: End() skips a buffer that was never created.
    spriteInstanceBuffer1.End();
    spriteInstanceBuffer0.End();
    instanceCount0 = 0;
    instanceCount1 = 0;
    instancedSpriteRenderers0 = {};
    instancedSpriteRenderers1 = {};
    spriteInstanceData0 = nullptr;
    spriteInstanceData1 = nullptr;

    atlasBuffer.End();
    perSpriteBuffer.End();
    perPassBuffer.End();
    spriteCount = 0;
    atlasCount = 0;

    for (uint32_t i = 0; i < spriteTextureCount; ++i) {
        spriteTextures[i].End();
    }
    spriteTextureCount = 0;

    device = VK_NULL_HANDLE;
    scene = nullptr;
}

void ME::SceneVulkan::UploadFrame(uint32_t frameIndex) {
    if (scene == nullptr) {
        return;
    }

    CBPerPass passData{};
    passData.viewMatrix = scene->spriteCamera->GetViewMatrix().GetDataForShader();
    passData.projectionMatrix = scene->spriteCamera->GetProjectionMatrix().GetDataForShader();
    passData.ambientLightData = scene->ambientLight->GetLightDataAmbient();
    passData.directionalLightData = scene->directionalLight->GetLightDataDirectional();
    perPassBuffer.CopyElement(frameIndex, 0, &passData);

    for (uint32_t i = 0; i < atlasCount; ++i) {
        atlasBuffer.CopyElement(frameIndex, i, &scene->textureAtlasProperties[i]);
    }

    for (uint32_t i = 0; i < spriteCount; ++i) {
        const ME::SpriteRenderer& sprite = spriteRenderers[i];
        CBPerSprite spriteData{};
        spriteData.modelMatrix = scene->spriteTransforms[i].GetModelMatrix().GetDataForShader();
        spriteData.color = sprite.color;
        spriteData.atlasIndex = sprite.atlasIndex;
        spriteData.flags = sprite.flags;
        perSpriteBuffer.CopyElement(frameIndex, i, &spriteData);
    }

    // The whole array in one copy (it is the buffer's one element), as DX12's CopyData in Draw.
    if (instanceCount0 != 0) {
        spriteInstanceBuffer0.CopyElement(frameIndex, 0, spriteInstanceData0);
    }
    if (instanceCount1 != 0) {
        spriteInstanceBuffer1.CopyElement(frameIndex, 0, spriteInstanceData1);
    }
}

uint32_t ME::SceneVulkan::GetPerPassOffset(uint32_t frameIndex) const {
    return perPassBuffer.GetOffset(frameIndex, 0);
}

uint32_t ME::SceneVulkan::GetSpriteOffset(uint32_t frameIndex, uint32_t spriteIndex) const {
    return perSpriteBuffer.GetOffset(frameIndex, spriteIndex);
}

uint32_t ME::SceneVulkan::GetAtlasOffset(uint32_t frameIndex, uint32_t atlasIndex) const {
    return atlasBuffer.GetOffset(frameIndex, atlasIndex);
}

uint32_t ME::SceneVulkan::GetInstanceOffset0(uint32_t frameIndex) const {
    return spriteInstanceBuffer0.GetOffset(frameIndex, 0);
}

uint32_t ME::SceneVulkan::GetInstanceOffset1(uint32_t frameIndex) const {
    return spriteInstanceBuffer1.GetOffset(frameIndex, 0);
}

bool ME::SceneVulkan::MakeSpriteTextures(VkPhysicalDevice physicalDevice, VkQueue queue, VkCommandPool uploadPool) {
    spriteTextureCount = static_cast<uint32_t>(scene->spriteTexturePaths.count);
    if (spriteTextureCount > Constants::MaxSpriteTextureCount) {
        ME::LogError("SceneVulkan: more sprite textures than MaxSpriteTextureCount; the extra ones are not loaded");
        spriteTextureCount = static_cast<uint32_t>(Constants::MaxSpriteTextureCount);
    }

    uint32_t loadedCount = 0;
    for (uint32_t i = 0; i < spriteTextureCount; ++i) {
        const char* path = scene->spriteTexturePaths[i];
        if (spriteTextures[i].Init(physicalDevice, device, queue, uploadPool, path)) {
            ++loadedCount;
        } else {
            ME::LogError("SceneVulkan: sprite texture ", i, " (", path, ") failed to load; its sprites are skipped");
            spriteTextures[i].End();  // frees whatever Init made before it failed
        }
    }

    ME::Log("SceneVulkan: ", loadedCount, " of ", spriteTextureCount, " sprite textures loaded");
    return true;
}

bool ME::SceneVulkan::MakeBuffers(VkPhysicalDevice physicalDevice) {
    if (!perPassBuffer.Init(physicalDevice, device, 1, sizeof(ME::CBPerPass), UploadBufferVulkan::Kind::Uniform)) {
        ME::LogError("SceneVulkan: failed to create the per-pass buffer");
        return false;
    }

    spriteCount = static_cast<uint32_t>(spriteRenderers.count);
    if (spriteCount > Constants::MaxSpriteRendererCount) {
        ME::LogError("SceneVulkan: more sprite renderers than MaxSpriteRendererCount; the extra ones are not drawn");
        spriteCount = static_cast<uint32_t>(Constants::MaxSpriteRendererCount);
    }
    // Sprite i reads spriteTransforms[i] too, so every drawn sprite needs one.
    if (spriteCount > scene->spriteTransforms.count) {
        ME::LogError("SceneVulkan: fewer sprite transforms than sprite renderers; the extra renderers are not drawn");
        spriteCount = static_cast<uint32_t>(scene->spriteTransforms.count);
    }
    // At least one element: set 1 always points at a buffer, even in a scene without sprites.
    if (!perSpriteBuffer.Init(physicalDevice, device, spriteCount > 0 ? spriteCount : 1, sizeof(ME::CBPerSprite),
                              UploadBufferVulkan::Kind::Uniform)) {
        ME::LogError("SceneVulkan: failed to create the per-sprite buffer");
        return false;
    }

    atlasCount = static_cast<uint32_t>(scene->textureAtlasProperties.count);
    if (atlasCount > Constants::MaxTextureAtlasPropertiesCount) {
        ME::LogError("SceneVulkan: more atlases than MaxTextureAtlasPropertiesCount; the extra ones are not used");
        atlasCount = static_cast<uint32_t>(Constants::MaxTextureAtlasPropertiesCount);
    }
    if (!atlasBuffer.Init(physicalDevice, device, atlasCount > 0 ? atlasCount : 1, sizeof(ME::TextureAtlasProperties),
                          UploadBufferVulkan::Kind::Uniform)) {
        ME::LogError("SceneVulkan: failed to create the atlas properties buffer");
        return false;
    }

    return true;
}

bool ME::SceneVulkan::MakeDescriptorSets(VkDescriptorSetLayout perPassLayout, VkDescriptorSetLayout perSpriteLayout,
                                         VkDescriptorSetLayout instancedLayout, VkDescriptorSetLayout textureLayout) {
    uint32_t loadedTextureCount = 0;
    for (uint32_t i = 0; i < spriteTextureCount; ++i) {
        if (spriteTextures[i].GetImageView() != VK_NULL_HANDLE) {
            ++loadedTextureCount;
        }
    }
    const uint32_t instancedSetCount = (instanceCount0 != 0 ? 1 : 0) + (instanceCount1 != 0 ? 1 : 0);

    // Totals over every set allocated below: set 0 has 1 dynamic UBO, set 1 has 2, each instanced set 1 dynamic SSBO
    // and 1 dynamic UBO, each texture set 1 image+sampler. A pool size of 0 descriptors is invalid, so those are left
    // out.
    VkDescriptorPoolSize poolSizes[3] = {};
    uint32_t poolSizeCount = 0;
    poolSizes[poolSizeCount++] = {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 3 + instancedSetCount};
    if (instancedSetCount > 0) {
        poolSizes[poolSizeCount++] = {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, instancedSetCount};
    }
    if (loadedTextureCount > 0) {
        poolSizes[poolSizeCount++] = {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, loadedTextureCount};
    }

    VkDescriptorPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.maxSets = 2 + instancedSetCount + loadedTextureCount;
    poolInfo.poolSizeCount = poolSizeCount;
    poolInfo.pPoolSizes = poolSizes;
    VkResult result = vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptorPool);
    if (result != VK_SUCCESS) {
        ME::LogError("SceneVulkan: vkCreateDescriptorPool failed (", static_cast<int>(result), ")");
        return false;
    }

    VkDescriptorSetAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = descriptorPool;
    allocInfo.descriptorSetCount = 1;

    allocInfo.pSetLayouts = &perPassLayout;
    result = vkAllocateDescriptorSets(device, &allocInfo, &perPassSet);
    if (result != VK_SUCCESS) {
        ME::LogError("SceneVulkan: vkAllocateDescriptorSets (per pass) failed (", static_cast<int>(result), ")");
        return false;
    }
    allocInfo.pSetLayouts = &perSpriteLayout;
    result = vkAllocateDescriptorSets(device, &allocInfo, &perSpriteSet);
    if (result != VK_SUCCESS) {
        ME::LogError("SceneVulkan: vkAllocateDescriptorSets (per sprite) failed (", static_cast<int>(result), ")");
        return false;
    }

    // One element each: the dynamic offset picks the frame's copy and the sprite or atlas.
    VkDescriptorBufferInfo passInfo = {perPassBuffer.GetBuffer(), 0, perPassBuffer.GetElementSize()};
    VkDescriptorBufferInfo spriteInfo = {perSpriteBuffer.GetBuffer(), 0, perSpriteBuffer.GetElementSize()};
    VkDescriptorBufferInfo atlasInfo = {atlasBuffer.GetBuffer(), 0, atlasBuffer.GetElementSize()};

    VkWriteDescriptorSet writes[3] = {};
    writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writes[0].dstSet = perPassSet;
    writes[0].dstBinding = 0;
    writes[0].descriptorCount = 1;
    writes[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    writes[0].pBufferInfo = &passInfo;
    writes[1] = writes[0];
    writes[1].dstSet = perSpriteSet;
    writes[1].pBufferInfo = &spriteInfo;
    writes[2] = writes[1];
    writes[2].dstBinding = 1;
    writes[2].pBufferInfo = &atlasInfo;
    vkUpdateDescriptorSets(device, 3, writes, 0, nullptr);

    // Instanced sets, each on its own: binding 0 the whole instance array, binding 1 the atlas properties (the
    // dynamic offsets pick the frame's copy and the atlas). DX12: CreateFrameSRVsInstanceData.
    allocInfo.pSetLayouts = &instancedLayout;
    if (instanceCount0 != 0 &&
        !WriteInstanceSet(allocInfo, spriteInstanceBuffer0, atlasInfo, &spriteInstanceSet0, "instanced set 0")) {
        return false;
    }
    if (instanceCount1 != 0 &&
        !WriteInstanceSet(allocInfo, spriteInstanceBuffer1, atlasInfo, &spriteInstanceSet1, "instanced set 1")) {
        return false;
    }

    allocInfo.pSetLayouts = &textureLayout;
    for (uint32_t i = 0; i < spriteTextureCount; ++i) {
        const VkImageView view = spriteTextures[i].GetImageView();
        if (view == VK_NULL_HANDLE) {
            continue;  // failed to load: textureSets[i] stays null and its sprites are skipped
        }
        result = vkAllocateDescriptorSets(device, &allocInfo, &textureSets[i]);
        if (result != VK_SUCCESS) {
            ME::LogError("SceneVulkan: vkAllocateDescriptorSets (texture ", i, ") failed (", static_cast<int>(result),
                         ")");
            return false;
        }

        VkDescriptorImageInfo imageInfo = {};
        imageInfo.sampler = VK_NULL_HANDLE;  // ignored: the layout's immutable sampler is used
        imageInfo.imageView = view;
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        VkWriteDescriptorSet write = {};
        write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.dstSet = textureSets[i];
        write.dstBinding = 0;
        write.descriptorCount = 1;
        write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        write.pImageInfo = &imageInfo;
        vkUpdateDescriptorSets(device, 1, &write, 0, nullptr);
    }

    ME::Log("SceneVulkan: descriptor sets created (", loadedTextureCount, " texture sets)");
    return true;
}

bool ME::SceneVulkan::WriteInstanceSet(const VkDescriptorSetAllocateInfo& allocInfo,
                                       const UploadBufferVulkan& instanceBuffer,
                                       const VkDescriptorBufferInfo& atlasInfo, VkDescriptorSet* outSet,
                                       const char* name) {
    const VkResult result = vkAllocateDescriptorSets(device, &allocInfo, outSet);
    if (result != VK_SUCCESS) {
        ME::LogError("SceneVulkan: vkAllocateDescriptorSets (", name, ") failed (", static_cast<int>(result), ")");
        return false;
    }

    // Range: the whole array, which the shader sees as the length of instances[].
    const VkDescriptorBufferInfo instanceInfo = {instanceBuffer.GetBuffer(), 0, instanceBuffer.GetElementSize()};

    VkWriteDescriptorSet writes[2] = {};
    writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writes[0].dstSet = *outSet;
    writes[0].dstBinding = 0;
    writes[0].descriptorCount = 1;
    writes[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
    writes[0].pBufferInfo = &instanceInfo;
    writes[1] = writes[0];
    writes[1].dstBinding = 1;
    writes[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    writes[1].pBufferInfo = &atlasInfo;
    vkUpdateDescriptorSets(device, 2, writes, 0, nullptr);
    return true;
}

bool ME::SceneVulkan::MakeSpriteInstanceBuffer(VkPhysicalDevice physicalDevice) {
    instanceCount0 = static_cast<uint32_t>(instancedSpriteRenderers0.count);
    if (instanceCount0 > Constants::MaxInstancedSpriteRendererCount) {
        ME::LogError(
            "SceneVulkan: more instanced sprites in set 0 than MaxInstancedSpriteRendererCount; the extra ones are not "
            "drawn");
        instanceCount0 = static_cast<uint32_t>(Constants::MaxInstancedSpriteRendererCount);
    }
    if (instanceCount0 != 0 && spriteInstanceData0 != nullptr) {
        if (!spriteInstanceBuffer0.Init(physicalDevice, device, 1,
                                        instanceCount0 * sizeof(ME::SpriteRendererInstanceData),
                                        UploadBufferVulkan::Kind::Storage)) {
            ME::LogError("SceneVulkan: failed to create instance buffer 0");
            return false;
        }
    } else {
        instanceCount0 = 0;
    }

    instanceCount1 = static_cast<uint32_t>(instancedSpriteRenderers1.count);
    if (instanceCount1 > Constants::MaxInstancedSpriteRendererCount) {
        ME::LogError(
            "SceneVulkan: more instanced sprites in set 1 than MaxInstancedSpriteRendererCount; the extra ones are not "
            "drawn");
        instanceCount1 = static_cast<uint32_t>(Constants::MaxInstancedSpriteRendererCount);
    }
    if (instanceCount1 != 0 && spriteInstanceData1 != nullptr) {
        if (!spriteInstanceBuffer1.Init(physicalDevice, device, 1,
                                        instanceCount1 * sizeof(ME::SpriteRendererInstanceData),
                                        UploadBufferVulkan::Kind::Storage)) {
            ME::LogError("SceneVulkan: failed to create instance buffer 1");
            return false;
        }
    } else {
        instanceCount1 = 0;
    }

    return true;
}

#endif  // ME_VULKAN
