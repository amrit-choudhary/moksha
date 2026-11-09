#pragma once
#ifdef ME_VULKAN

/**
 * Vulkan's quad class, the counterpart of QuadDX and QuadMetal: the shared Quad's vertices and 16-bit indices in GPU
 * buffers.
 */

#include <vulkan/vulkan.h>

#include "moksha/client/rendering/shared/quad.h"

namespace ME {

class QuadVulkan : public Quad {
   public:
    QuadVulkan() = default;
    ~QuadVulkan() override = default;

    QuadVulkan(const QuadVulkan&) = delete;
    QuadVulkan& operator=(const QuadVulkan&) = delete;

    // Creates the vertex and index buffers from the Quad's data. False (logged) on failure; call End() either way.
    bool Init(VkPhysicalDevice physicalDevice, VkDevice inDevice);
    // Frees both buffers. The GPU must be done with them (call after vkDeviceWaitIdle).
    void End();

    VkBuffer GetVertexBuffer() const;
    VkBuffer GetIndexBuffer() const;

   private:
    VkDevice device = VK_NULL_HANDLE;
    VkBuffer vertexBuffer = VK_NULL_HANDLE;
    VkDeviceMemory vertexMemory = VK_NULL_HANDLE;
    VkBuffer indexBuffer = VK_NULL_HANDLE;
    VkDeviceMemory indexMemory = VK_NULL_HANDLE;
};

}  // namespace ME

#endif  // ME_VULKAN
