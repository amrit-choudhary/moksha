#pragma once
#ifdef ME_VULKAN

/**
 * Vulkan utility functions, used by RendererVulkan and ShaderVulkan. The functions are not part of the public API.
 */

#include <vulkan/vulkan.h>

#include <cstdint>

namespace ME {

namespace VulkanUtils {

// Returns a string for the device type, for logging.
const char* DeviceTypeName(VkPhysicalDeviceType type);

// Finds a memory type that matches the typeBits and has the wanted properties. Returns UINT32_MAX if none found.
uint32_t FindMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeBits, VkMemoryPropertyFlags wantedProps);

// Creates a size-byte buffer for usage in memory the CPU can write: GPU-fast shared memory when the GPU has it,
// else plain host-visible memory. Copies data in when it isn't null. False (logged) on failure, with both outputs
// null.
bool CreateBuffer(VkPhysicalDevice physicalDevice, VkDevice device, VkDeviceSize size, VkBufferUsageFlags usage,
                  const void* data, VkBuffer* outBuffer, VkDeviceMemory* outMemory);

// Destroys a buffer made by CreateBuffer and frees its memory; nulls both. Safe on null handles.
void DestroyBuffer(VkDevice device, VkBuffer* buffer, VkDeviceMemory* memory);

}  // namespace VulkanUtils

}  // namespace ME

#endif  // ME_VULKAN
