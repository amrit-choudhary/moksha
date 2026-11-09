#ifdef ME_VULKAN

#include "utils_vulkan.h"

#include <cstddef>
#include <cstring>

#include "moksha/logging/logging.h"

const char* ME::VulkanUtils::DeviceTypeName(VkPhysicalDeviceType type) {
    switch (type) {
        case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
            return "discrete";
        case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
            return "integrated";
        case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
            return "virtual";
        case VK_PHYSICAL_DEVICE_TYPE_CPU:
            return "cpu";
        default:
            return "other";
    }
}

uint32_t ME::VulkanUtils::FindMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeBits,
                                         VkMemoryPropertyFlags wantedProps) {
    VkPhysicalDeviceMemoryProperties props = {};
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &props);

    for (uint32_t i = 0; i < props.memoryTypeCount; ++i) {
        const bool allowed = (typeBits & (1u << i)) != 0;
        const bool hasFlags = (props.memoryTypes[i].propertyFlags & wantedProps) == wantedProps;
        if (allowed && hasFlags) {
            return i;
        }
    }
    return UINT32_MAX;
}

bool ME::VulkanUtils::CreateBuffer(VkPhysicalDevice physicalDevice, VkDevice device, VkDeviceSize size,
                                   VkBufferUsageFlags usage, const void* data, VkBuffer* outBuffer,
                                   VkDeviceMemory* outMemory) {
    *outBuffer = VK_NULL_HANDLE;
    *outMemory = VK_NULL_HANDLE;

    VkBufferCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    info.size = size;
    info.usage = usage;
    info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    VkResult result = vkCreateBuffer(device, &info, nullptr, outBuffer);
    if (result < 0) {
        ME::LogError("VulkanUtils: vkCreateBuffer failed (", static_cast<int>(result), ")");
        return false;
    }

    VkMemoryRequirements req = {};
    vkGetBufferMemoryRequirements(device, *outBuffer, &req);

    const VkMemoryPropertyFlags hostWrite = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    uint32_t typeIndex =
        FindMemoryType(physicalDevice, req.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | hostWrite);
    if (typeIndex == UINT32_MAX) {
        typeIndex = FindMemoryType(physicalDevice, req.memoryTypeBits, hostWrite);
    }
    if (typeIndex == UINT32_MAX) {
        ME::LogError("VulkanUtils: no host-visible memory type for buffer");
        DestroyBuffer(device, outBuffer, outMemory);
        return false;
    }

    VkMemoryAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = req.size;
    allocInfo.memoryTypeIndex = typeIndex;
    result = vkAllocateMemory(device, &allocInfo, nullptr, outMemory);
    if (result < 0) {
        ME::LogError("VulkanUtils: vkAllocateMemory failed (", static_cast<int>(result), ")");
        DestroyBuffer(device, outBuffer, outMemory);
        return false;
    }

    result = vkBindBufferMemory(device, *outBuffer, *outMemory, 0);
    if (result < 0) {
        ME::LogError("VulkanUtils: vkBindBufferMemory failed (", static_cast<int>(result), ")");
        DestroyBuffer(device, outBuffer, outMemory);
        return false;
    }

    // Coherent memory: the writes reach the GPU without a flush.
    if (data != nullptr) {
        void* mapped = nullptr;
        result = vkMapMemory(device, *outMemory, 0, size, 0, &mapped);
        if (result < 0) {
            ME::LogError("VulkanUtils: vkMapMemory failed (", static_cast<int>(result), ")");
            DestroyBuffer(device, outBuffer, outMemory);
            return false;
        }
        std::memcpy(mapped, data, static_cast<size_t>(size));
        vkUnmapMemory(device, *outMemory);
    }

    return true;
}

void ME::VulkanUtils::DestroyBuffer(VkDevice device, VkBuffer* buffer, VkDeviceMemory* memory) {
    if (*buffer != VK_NULL_HANDLE) {
        vkDestroyBuffer(device, *buffer, nullptr);
        *buffer = VK_NULL_HANDLE;
    }
    if (*memory != VK_NULL_HANDLE) {
        vkFreeMemory(device, *memory, nullptr);
        *memory = VK_NULL_HANDLE;
    }
}

bool ME::VulkanUtils::BeginOneTimeCommands(VkDevice device, VkCommandPool pool, VkCommandBuffer* outCmd) {
    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = pool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;
    VkResult result = vkAllocateCommandBuffers(device, &allocInfo, outCmd);
    if (result < 0) {
        ME::LogError("VulkanUtils: vkAllocateCommandBuffers failed (", static_cast<int>(result), ")");
        return false;
    }

    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    result = vkBeginCommandBuffer(*outCmd, &beginInfo);
    if (result < 0) {
        ME::LogError("VulkanUtils: vkBeginCommandBuffer failed (", static_cast<int>(result), ")");
        vkFreeCommandBuffers(device, pool, 1, outCmd);
        *outCmd = VK_NULL_HANDLE;
        return false;
    }

    return true;
}

bool ME::VulkanUtils::EndOneTimeCommands(VkDevice device, VkQueue queue, VkCommandPool pool, VkCommandBuffer cmd) {
    // The buffer goes back to the pool on every path.
    VkResult result = vkEndCommandBuffer(cmd);
    if (result < 0) {
        ME::LogError("VulkanUtils: vkEndCommandBuffer failed (", static_cast<int>(result), ")");
        vkFreeCommandBuffers(device, pool, 1, &cmd);
        return false;
    }

    VkSubmitInfo submit = {};
    submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit.commandBufferCount = 1;
    submit.pCommandBuffers = &cmd;
    result = vkQueueSubmit(queue, 1, &submit, VK_NULL_HANDLE);
    if (result < 0) {
        ME::LogError("VulkanUtils: vkQueueSubmit failed (", static_cast<int>(result), ")");
        vkFreeCommandBuffers(device, pool, 1, &cmd);
        return false;
    }

    // Waits for the GPU, so the commands and anything they read may be freed afterwards.
    result = vkQueueWaitIdle(queue);
    vkFreeCommandBuffers(device, pool, 1, &cmd);
    if (result < 0) {
        ME::LogError("VulkanUtils: vkQueueWaitIdle failed (", static_cast<int>(result), ")");
        return false;
    }
    return true;
}

#endif  // ME_VULKAN
