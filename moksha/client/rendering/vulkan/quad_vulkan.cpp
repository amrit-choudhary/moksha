#ifdef ME_VULKAN

#include "quad_vulkan.h"

#include "utils_vulkan.h"

bool ME::QuadVulkan::Init(VkPhysicalDevice physicalDevice, VkDevice inDevice) {
    device = inDevice;

    // The CPU writes both through a mapping, so no transfer usage is needed.
    if (!ME::VulkanUtils::CreateBuffer(physicalDevice, device, verticesSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                                       vertices, &vertexBuffer, &vertexMemory)) {
        return false;
    }

    if (!ME::VulkanUtils::CreateBuffer(physicalDevice, device, indicesSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, indices,
                                       &indexBuffer, &indexMemory)) {
        return false;
    }

    return true;
}

void ME::QuadVulkan::End() {
    ME::VulkanUtils::DestroyBuffer(device, &indexBuffer, &indexMemory);
    ME::VulkanUtils::DestroyBuffer(device, &vertexBuffer, &vertexMemory);
}

VkBuffer ME::QuadVulkan::GetVertexBuffer() const {
    return vertexBuffer;
}

VkBuffer ME::QuadVulkan::GetIndexBuffer() const {
    return indexBuffer;
}

#endif  // ME_VULKAN
