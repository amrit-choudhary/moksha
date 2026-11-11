#ifdef ME_VULKAN

#include "scene_ui_vulkan.h"

#include "moksha/client/rendering/shared/shader_constants.h"
#include "moksha/client/rendering/shared/sprite_renderer.h"
#include "moksha/client/rendering/shared/text_renderer.h"
#include "moksha/logging/logging.h"

bool ME::SceneUIVulkan::Init(VkPhysicalDevice physicalDevice, VkDevice inDevice, VkQueue queue,
                             VkCommandPool uploadPool, VkDescriptorSetLayout perPassLayout,
                             VkDescriptorSetLayout instancedLayout, VkDescriptorSetLayout textureLayout,
                             ME::SceneUI* uiScene) {
    device = inDevice;
    scene = uiScene;

    if (!MakeSpriteTextures(physicalDevice, queue, uploadPool)) {
        return false;
    }

    if (!MakeBuffers(physicalDevice)) {
        return false;
    }

    if (!MakeDescriptorSets(perPassLayout, instancedLayout, textureLayout)) {
        return false;
    }

    Update();
    ME::Log("SceneUIVulkan: ", spriteTextureCount, " textures, ", atlasCount, " atlases, ", uiSpriteInstanceCount,
            " UI sprites, ", textInstanceCount, " glyphs");
    return true;
}

void ME::SceneUIVulkan::End() {
    if (descriptorPool != VK_NULL_HANDLE) {
        vkDestroyDescriptorPool(device, descriptorPool, nullptr);  // frees every set made from it
        descriptorPool = VK_NULL_HANDLE;
    }
    uiSpritePassSet = VK_NULL_HANDLE;
    textPassSet = VK_NULL_HANDLE;
    uiSpriteInstanceSet = VK_NULL_HANDLE;
    textInstanceSet = VK_NULL_HANDLE;
    for (uint32_t i = 0; i < Constants::MaxSpriteTextureCount; ++i) {
        textureSets[i] = VK_NULL_HANDLE;
    }

    // Harmless for a buffer that was never created: End() skips null handles.
    textInstanceBuffer.End();
    uiSpriteInstanceBuffer.End();
    atlasBuffer.End();
    textPassBuffer.End();
    uiSpritePassBuffer.End();
    uiSpriteInstanceCount = 0;
    textInstanceCount = 0;
    atlasCount = 0;

    for (uint32_t i = 0; i < spriteTextureCount; ++i) {
        spriteTextures[i].End();
    }
    spriteTextureCount = 0;

    device = VK_NULL_HANDLE;
    scene = nullptr;
}

void ME::SceneUIVulkan::Update() {
    if (scene == nullptr) {
        return;
    }
    // The live counts change as UI elements are added or rebuilt; the arrays themselves are SceneUI's and stay put.
    uiSpriteInstanceCount = static_cast<uint32_t>(scene->uiSpriteInstanceData.count);
    if (uiSpriteInstanceCount > Constants::MaxUISpriteInstanceDataCount) {
        uiSpriteInstanceCount = static_cast<uint32_t>(Constants::MaxUISpriteInstanceDataCount);
    }
    textInstanceCount = static_cast<uint32_t>(scene->textInstanceData.count);
    if (textInstanceCount > Constants::MaxTextInstanceDataCount) {
        textInstanceCount = static_cast<uint32_t>(Constants::MaxTextInstanceDataCount);
    }
    if (scene->uiSpriteInstanceData.data == nullptr) {
        uiSpriteInstanceCount = 0;
    }
    if (scene->textInstanceData.data == nullptr) {
        textInstanceCount = 0;
    }
}

void ME::SceneUIVulkan::UploadFrame(uint32_t frameIndex, uint32_t screenDimension) {
    if (scene == nullptr) {
        return;
    }

    CBPerPassUISprite spritePassData{};
    spritePassData.screenDimension = screenDimension;
    uiSpritePassBuffer.CopyElement(frameIndex, 0, &spritePassData);
    CBPerPassUIText textPassData{};
    textPassData.screenDimension = screenDimension;
    textPassBuffer.CopyElement(frameIndex, 0, &textPassData);

    for (uint32_t i = 0; i < atlasCount; ++i) {
        atlasBuffer.CopyElement(frameIndex, i, &scene->textureAtlasProperties[i]);
    }

    // Only the live instances: the buffers are sized at capacity, as DX12's CopyElements.
    if (uiSpriteInstanceCount != 0) {
        uiSpriteInstanceBuffer.CopyElements(frameIndex, scene->uiSpriteInstanceData.data, uiSpriteInstanceCount);
    }
    if (textInstanceCount != 0) {
        textInstanceBuffer.CopyElements(frameIndex, scene->textInstanceData.data, textInstanceCount);
    }
}

uint32_t ME::SceneUIVulkan::GetUISpritePassOffset(uint32_t frameIndex) const {
    return uiSpritePassBuffer.GetOffset(frameIndex, 0);
}

uint32_t ME::SceneUIVulkan::GetTextPassOffset(uint32_t frameIndex) const {
    return textPassBuffer.GetOffset(frameIndex, 0);
}

uint32_t ME::SceneUIVulkan::GetAtlasOffset(uint32_t frameIndex, uint32_t atlasIndex) const {
    return atlasBuffer.GetOffset(frameIndex, atlasIndex);
}

uint32_t ME::SceneUIVulkan::GetUISpriteInstanceOffset(uint32_t frameIndex) const {
    return uiSpriteInstanceBuffer.GetOffset(frameIndex, 0);
}

uint32_t ME::SceneUIVulkan::GetTextInstanceOffset(uint32_t frameIndex) const {
    return textInstanceBuffer.GetOffset(frameIndex, 0);
}

bool ME::SceneUIVulkan::MakeSpriteTextures(VkPhysicalDevice physicalDevice, VkQueue queue, VkCommandPool uploadPool) {
    spriteTextureCount = static_cast<uint32_t>(scene->spriteTexturePaths.count);
    if (spriteTextureCount > Constants::MaxSpriteTextureCount) {
        ME::LogError("SceneUIVulkan: more textures than MaxSpriteTextureCount; the extra ones are not loaded");
        spriteTextureCount = static_cast<uint32_t>(Constants::MaxSpriteTextureCount);
    }

    for (uint32_t i = 0; i < spriteTextureCount; ++i) {
        const char* path = scene->spriteTexturePaths[i];
        if (!spriteTextures[i].Init(physicalDevice, device, queue, uploadPool, path)) {
            ME::LogError("SceneUIVulkan: texture ", i, " (", path, ") failed to load; its pass is skipped");
            spriteTextures[i].End();  // frees whatever Init made before it failed
        }
    }
    return true;
}

bool ME::SceneUIVulkan::MakeBuffers(VkPhysicalDevice physicalDevice) {
    if (!uiSpritePassBuffer.Init(physicalDevice, device, 1, sizeof(ME::CBPerPassUISprite),
                                 UploadBufferVulkan::Kind::Uniform) ||
        !textPassBuffer.Init(physicalDevice, device, 1, sizeof(ME::CBPerPassUIText),
                             UploadBufferVulkan::Kind::Uniform)) {
        ME::LogError("SceneUIVulkan: failed to create the pass constant buffers");
        return false;
    }

    atlasCount = static_cast<uint32_t>(scene->textureAtlasProperties.count);
    if (atlasCount > Constants::MaxTextureAtlasPropertiesCount) {
        ME::LogError("SceneUIVulkan: more atlases than MaxTextureAtlasPropertiesCount; the extra ones are not used");
        atlasCount = static_cast<uint32_t>(Constants::MaxTextureAtlasPropertiesCount);
    }
    // At least one element: the instanced sets always point at a buffer, even in a UI scene without atlases.
    if (!atlasBuffer.Init(physicalDevice, device, atlasCount > 0 ? atlasCount : 1, sizeof(ME::TextureAtlasProperties),
                          UploadBufferVulkan::Kind::Uniform)) {
        ME::LogError("SceneUIVulkan: failed to create the atlas properties buffer");
        return false;
    }

    // At capacity, so UI elements added after Init fit, as SceneUIDX.
    if (!uiSpriteInstanceBuffer.Init(physicalDevice, device,
                                     static_cast<uint32_t>(Constants::MaxUISpriteInstanceDataCount),
                                     sizeof(ME::UISpriteRendererInstanceData), UploadBufferVulkan::Kind::Storage)) {
        ME::LogError("SceneUIVulkan: failed to create the UI sprite instance buffer");
        return false;
    }
    if (!textInstanceBuffer.Init(physicalDevice, device, static_cast<uint32_t>(Constants::MaxTextInstanceDataCount),
                                 sizeof(ME::TextRendererInstanceData), UploadBufferVulkan::Kind::Storage)) {
        ME::LogError("SceneUIVulkan: failed to create the text instance buffer");
        return false;
    }

    return true;
}

bool ME::SceneUIVulkan::MakeDescriptorSets(VkDescriptorSetLayout perPassLayout, VkDescriptorSetLayout instancedLayout,
                                           VkDescriptorSetLayout textureLayout) {
    uint32_t loadedTextureCount = 0;
    for (uint32_t i = 0; i < spriteTextureCount; ++i) {
        if (spriteTextures[i].GetImageView() != VK_NULL_HANDLE) {
            ++loadedTextureCount;
        }
    }

    // Totals over every set allocated below: two set 0s with 1 dynamic UBO each, two set 1s with 1 dynamic SSBO and
    // 1 dynamic UBO each, each texture set 1 image+sampler. A pool size of 0 descriptors is invalid, so it is left out.
    VkDescriptorPoolSize poolSizes[3] = {};
    uint32_t poolSizeCount = 0;
    poolSizes[poolSizeCount++] = {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 4};
    poolSizes[poolSizeCount++] = {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 2};
    if (loadedTextureCount > 0) {
        poolSizes[poolSizeCount++] = {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, loadedTextureCount};
    }

    VkDescriptorPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.maxSets = 4 + loadedTextureCount;
    poolInfo.poolSizeCount = poolSizeCount;
    poolInfo.pPoolSizes = poolSizes;
    VkResult result = vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptorPool);
    if (result != VK_SUCCESS) {
        ME::LogError("SceneUIVulkan: vkCreateDescriptorPool failed (", static_cast<int>(result), ")");
        return false;
    }

    VkDescriptorSetAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = descriptorPool;
    allocInfo.descriptorSetCount = 1;

    VkDescriptorSet* const passSets[2] = {&uiSpritePassSet, &textPassSet};
    VkDescriptorSet* const instanceSets[2] = {&uiSpriteInstanceSet, &textInstanceSet};
    for (uint32_t i = 0; i < 2; ++i) {
        allocInfo.pSetLayouts = &perPassLayout;
        result = vkAllocateDescriptorSets(device, &allocInfo, passSets[i]);
        if (result != VK_SUCCESS) {
            ME::LogError("SceneUIVulkan: vkAllocateDescriptorSets (pass ", i, ") failed (", static_cast<int>(result),
                         ")");
            return false;
        }
        allocInfo.pSetLayouts = &instancedLayout;
        result = vkAllocateDescriptorSets(device, &allocInfo, instanceSets[i]);
        if (result != VK_SUCCESS) {
            ME::LogError("SceneUIVulkan: vkAllocateDescriptorSets (instances ", i, ") failed (",
                         static_cast<int>(result), ")");
            return false;
        }
    }

    // Instance ranges cover the whole capacity: the shader indexes only the drawn instances.
    const VkDescriptorBufferInfo spritePassInfo = {uiSpritePassBuffer.GetBuffer(), 0,
                                                   uiSpritePassBuffer.GetDescriptorRange()};
    const VkDescriptorBufferInfo textPassInfo = {textPassBuffer.GetBuffer(), 0, textPassBuffer.GetDescriptorRange()};
    const VkDescriptorBufferInfo atlasInfo = {atlasBuffer.GetBuffer(), 0, atlasBuffer.GetDescriptorRange()};
    const VkDescriptorBufferInfo spriteInstanceInfo = {uiSpriteInstanceBuffer.GetBuffer(), 0,
                                                       uiSpriteInstanceBuffer.GetDescriptorRange()};
    const VkDescriptorBufferInfo textInstanceInfo = {textInstanceBuffer.GetBuffer(), 0,
                                                     textInstanceBuffer.GetDescriptorRange()};

    VkWriteDescriptorSet writes[6] = {};
    for (VkWriteDescriptorSet& write : writes) {
        write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.descriptorCount = 1;
        write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    }
    writes[0].dstSet = uiSpritePassSet;
    writes[0].pBufferInfo = &spritePassInfo;
    writes[1].dstSet = textPassSet;
    writes[1].pBufferInfo = &textPassInfo;
    writes[2].dstSet = uiSpriteInstanceSet;
    writes[2].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
    writes[2].pBufferInfo = &spriteInstanceInfo;
    writes[3].dstSet = uiSpriteInstanceSet;
    writes[3].dstBinding = 1;
    writes[3].pBufferInfo = &atlasInfo;
    writes[4].dstSet = textInstanceSet;
    writes[4].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
    writes[4].pBufferInfo = &textInstanceInfo;
    writes[5].dstSet = textInstanceSet;
    writes[5].dstBinding = 1;
    writes[5].pBufferInfo = &atlasInfo;
    vkUpdateDescriptorSets(device, 6, writes, 0, nullptr);

    allocInfo.pSetLayouts = &textureLayout;
    for (uint32_t i = 0; i < spriteTextureCount; ++i) {
        const VkImageView view = spriteTextures[i].GetImageView();
        if (view == VK_NULL_HANDLE) {
            continue;  // failed to load: textureSets[i] stays null and its pass is skipped
        }
        result = vkAllocateDescriptorSets(device, &allocInfo, &textureSets[i]);
        if (result != VK_SUCCESS) {
            ME::LogError("SceneUIVulkan: vkAllocateDescriptorSets (texture ", i, ") failed (", static_cast<int>(result),
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

    return true;
}

#endif  // ME_VULKAN
