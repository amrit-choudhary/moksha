#ifdef ME_VULKAN

#include "renderer_vulkan.h"

#include <cstring>

#include "moksha/client/rendering/shared/shader_constants.h"
#include "moksha/client/rendering/shared/sprite_renderer.h"
#include "moksha/logging/logging.h"
#include "moksha/shared/math/math.h"
#include "pipeline_layout_vulkan.h"
#include "pso_vulkan.h"
#include "sampler_vulkan.h"
#include "utils_vulkan.h"
#include "vulkan_view.h"

namespace {

const char* const kValidationLayer = "VK_LAYER_KHRONOS_validation";
constexpr uint32_t kMaxInstanceExtensions = 8;
constexpr uint32_t kMaxPhysicalDevices = 8;
constexpr uint32_t kMaxQueueFamilies = 16;
constexpr uint32_t kMaxSurfaceFormats = 32;

// True if the loader finds the validation layer. Used for debug.
bool HasValidationLayer() {
    VkLayerProperties layers[64] = {};
    uint32_t layerCount = 64;
    if (vkEnumerateInstanceLayerProperties(&layerCount, layers) < 0) {
        return false;
    }
    for (uint32_t i = 0; i < layerCount; ++i) {
        if (std::strcmp(layers[i].layerName, kValidationLayer) == 0) {
            return true;
        }
    }
    return false;
}

// Sends validation messages to the engine log. VK_FALSE lets the call that caused the message carry on.
VKAPI_ATTR VkBool32 VKAPI_CALL OnVulkanMessage(VkDebugUtilsMessageSeverityFlagBitsEXT severity,
                                               VkDebugUtilsMessageTypeFlagsEXT /*types*/,
                                               const VkDebugUtilsMessengerCallbackDataEXT* data, void* /*userData*/) {
    if (severity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
        ME::LogError("Vulkan: ", data->pMessage);
    } else {
        ME::LogWarning("Vulkan: ", data->pMessage);
    }
    return VK_FALSE;
}

// Which messages reach OnVulkanMessage: warnings and errors of every type.
VkDebugUtilsMessengerCreateInfoEXT MakeMessengerCreateInfo() {
    VkDebugUtilsMessengerCreateInfoEXT info = {};
    info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    info.messageSeverity =
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                       VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    info.pfnUserCallback = OnVulkanMessage;
    return info;
}

}  // namespace

bool ME::RendererVulkan::InitVulkan(const VulkanView* inView, const WindowState* inWindow) {
    view = inView;
    window = inWindow;
    ME::Log("RendererVulkan::Init (skeleton: nothing is drawn yet)");

    if (!CreateInstance()) {
        ME::LogError("RendererVulkan: failed to create Vulkan instance");
        return false;
    }

    if (!SetupDebugMessenger()) {
        ME::LogError("RendererVulkan: failed to setup debug messenger");
        return false;
    }

    if (!CreateSurface()) {
        ME::LogError("RendererVulkan: failed to create surface");
        return false;
    }

    if (!PickPhysicalDevice()) {
        ME::LogError("RendererVulkan: failed to pick a physical device");
        return false;
    }

    if (!PickQueueFamily()) {
        ME::LogError("RendererVulkan: failed to pick a queue family");
        return false;
    }

    if (!CreateLogicalDevice()) {
        ME::LogError("RendererVulkan: failed to create logical device");
        return false;
    }

    if (!CreateSwapchain()) {
        ME::LogError("RendererVulkan: failed to create swapchain");
        return false;
    }

    if (!CreateSwapchainImageViews()) {
        ME::LogError("RendererVulkan: failed to create image views");
        return false;
    }

    if (!CreateRenderPass()) {
        ME::LogError("RendererVulkan: failed to create render pass");
        return false;
    }

    if (!CreateSwapchainFramebuffers()) {
        ME::LogError("RendererVulkan: failed to create swapchain framebuffers");
        return false;
    }

    if (!CreateCommandBuffers()) {
        ME::LogError("RendererVulkan: failed to create command buffers");
        return false;
    }

    if (!CreateSyncObjects()) {
        ME::LogError("RendererVulkan: failed to create sync objects");
        return false;
    }

    if (!quad.Init(physicalDevice, device)) {
        ME::LogError("RendererVulkan: failed to create quad");
        return false;
    }

    // Before the pipelines: their set layouts hold the samplers.
    samplerPointClamp = ME::SamplerVulkan::CreatePointClamp(device);
    if (samplerPointClamp == VK_NULL_HANDLE) {
        ME::LogError("RendererVulkan: failed to create the point clamp sampler");
        return false;
    }

    if (!CreatePSO()) {
        ME::LogError("RendererVulkan: failed to create the 2D atlas pipeline");
        return false;
    }

    if (!testTexture.Init(physicalDevice, device, graphicsQueue, uploadCommandPool, "textures/ui/ui_atlas.dds")) {
        ME::LogError("RendererVulkan: failed to initialize the test texture");
        return false;
    }

    if (!CreateDescriptorSets()) {
        ME::LogError("RendererVulkan: failed to create descriptor sets");
        return false;
    }

    return true;
}

void ME::RendererVulkan::SetScenes(ME::Scene* gameScene, ME::SceneUI* inUiScene) {
    scene = gameScene;
    uiScene = inUiScene;
}

void ME::RendererVulkan::SetDebugUIScene(ME::SceneUI* inDebugUiScene) {
    debugUiScene = inDebugUiScene;
}

void ME::RendererVulkan::Update() {
    if (scene != nullptr) {
        scene->Update();
    }
    if (uiScene != nullptr) {
        uiScene->Update();
    }
    if (debugUiScene != nullptr) {
        debugUiScene->Update();
    }
}

void ME::RendererVulkan::Draw() {
    VkCommandBuffer cmd = commandBuffers[frameIndex];
    VkFence fence = inFlightFences[frameIndex];

    vkWaitForFences(device, 1, &fence, VK_TRUE, UINT64_MAX);

    uint32_t imageIndex = 0;
    VkResult result = vkAcquireNextImageKHR(device, swapchain, UINT64_MAX, imageAvailableSemaphores[frameIndex],
                                            VK_NULL_HANDLE, &imageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        return;  // swapchain no longer matches the surface. TODO: recreate it
    }

    if (result < 0) {
        ME::LogError("RendererVulkan: vkAcquireNextImageKHR failed (", static_cast<int>(result), ")");
        return;
    }

    vkResetFences(device, 1, &fence);

    if (scene != nullptr) {
        CBPerPass passData{};
        passData.viewMatrix = scene->spriteCamera->GetViewMatrix().GetDataForShader();
        passData.projectionMatrix = scene->spriteCamera->GetProjectionMatrix().GetDataForShader();
        perPassBuffer.CopyElement(frameIndex, 0, &passData);
    }

    CBPerSprite spriteData{};
    spriteData.modelMatrix = ME::Mat4::Scale(ME::Vec4{200.0f, 200.0f, 1.0f, 1.0f}).GetDataForShader();
    perSpriteBuffer.CopyElement(frameIndex, 0, &spriteData);

    vkResetCommandPool(device, commandPools[frameIndex], 0);

    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkBeginCommandBuffer(cmd, &beginInfo);

    VkClearValue clearValue = {};
    clearValue.color = {{0.01f, 0.01f, 0.01f, 1.0f}};  // same as RendererDX

    VkRenderPassBeginInfo passInfo = {};
    passInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    passInfo.renderPass = renderPass;
    passInfo.framebuffer = swapchainFramebuffers[imageIndex];
    passInfo.renderArea.offset = {0, 0};
    passInfo.renderArea.extent = swapchainExtent;
    passInfo.clearValueCount = 1;
    passInfo.pClearValues = &clearValue;

    vkCmdBeginRenderPass(cmd, &passInfo, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pso2DAtl);

    const VkDescriptorSet sets[3] = {perPassSet, perSpriteSet, textureSet};
    const uint32_t dynamicOffsets[2] = {perPassBuffer.GetOffset(frameIndex, 0),
                                        perSpriteBuffer.GetOffset(frameIndex, 0)};
    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, layout2DAtl, 0, 3, sets, 2, dynamicOffsets);

    VkViewport viewport = {};
    viewport.x = 0.0f;
    viewport.y = static_cast<float>(swapchainExtent.height);
    viewport.width = static_cast<float>(swapchainExtent.width);
    viewport.height = -static_cast<float>(swapchainExtent.height);

    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(cmd, 0, 1, &viewport);

    VkRect2D scissor = {};
    scissor.offset = {0, 0};
    scissor.extent = swapchainExtent;
    vkCmdSetScissor(cmd, 0, 1, &scissor);

    const VkBuffer vertexBuffer = quad.GetVertexBuffer();
    const VkDeviceSize vertexOffset = 0;
    vkCmdBindVertexBuffers(cmd, 0, 1, &vertexBuffer, &vertexOffset);
    vkCmdBindIndexBuffer(cmd, quad.GetIndexBuffer(), 0, VK_INDEX_TYPE_UINT16);

    vkCmdDrawIndexed(cmd, quad.indexCount, 1, 0, 0, 0);

    vkCmdEndRenderPass(cmd);

    result = vkEndCommandBuffer(cmd);
    if (result < 0) {
        ME::LogError("RendererVulkan: vkEndCommandBuffer failed (", static_cast<int>(result), ")");
        return;
    }

    const VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

    VkSubmitInfo submit = {};
    submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit.waitSemaphoreCount = 1;
    submit.pWaitSemaphores = &imageAvailableSemaphores[frameIndex];
    submit.pWaitDstStageMask = &waitStage;
    submit.commandBufferCount = 1;
    submit.pCommandBuffers = &cmd;
    submit.signalSemaphoreCount = 1;
    submit.pSignalSemaphores = &renderFinishedSemaphores[imageIndex];

    result = vkQueueSubmit(graphicsQueue, 1, &submit, fence);

    if (result < 0) {
        ME::LogError("RendererVulkan: vkQueueSubmit failed (", static_cast<int>(result), ")");
        return;
    }

    VkPresentInfoKHR present = {};
    present.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present.waitSemaphoreCount = 1;
    present.pWaitSemaphores = &renderFinishedSemaphores[imageIndex];
    present.swapchainCount = 1;
    present.pSwapchains = &swapchain;
    present.pImageIndices = &imageIndex;

    result = vkQueuePresentKHR(graphicsQueue, &present);
    if (result < 0 && result != VK_ERROR_OUT_OF_DATE_KHR) {
        ME::LogError("RendererVulkan: vkQueuePresentKHR failed (", static_cast<int>(result), ")");
    }
    // OUT_OF_DATE / SUBOPTIMAL: carry on. TODO: recreate the swapchain.

    // The slot was submitted, so it always advances.
    frameIndex = (frameIndex + 1) % static_cast<uint32_t>(Constants::FramesInFlight);
}

void ME::RendererVulkan::End() {
    if (device != VK_NULL_HANDLE) {
        vkDeviceWaitIdle(device);
        if (descriptorPool != VK_NULL_HANDLE) {
            vkDestroyDescriptorPool(device, descriptorPool, nullptr);  // frees the sets too
            descriptorPool = VK_NULL_HANDLE;
        }
        perPassSet = VK_NULL_HANDLE;
        perSpriteSet = VK_NULL_HANDLE;
        textureSet = VK_NULL_HANDLE;
        perSpriteBuffer.End();
        perPassBuffer.End();
        testTexture.End();
        if (pso2DAtl != VK_NULL_HANDLE) {
            vkDestroyPipeline(device, pso2DAtl, nullptr);
            pso2DAtl = VK_NULL_HANDLE;
        }
        if (layout2DAtl != VK_NULL_HANDLE) {
            vkDestroyPipelineLayout(device, layout2DAtl, nullptr);
            layout2DAtl = VK_NULL_HANDLE;
        }
        if (setLayoutTexture != VK_NULL_HANDLE) {
            vkDestroyDescriptorSetLayout(device, setLayoutTexture, nullptr);
            setLayoutTexture = VK_NULL_HANDLE;
        }
        if (setLayoutPerSprite != VK_NULL_HANDLE) {
            vkDestroyDescriptorSetLayout(device, setLayoutPerSprite, nullptr);
            setLayoutPerSprite = VK_NULL_HANDLE;
        }
        if (setLayoutPerPass != VK_NULL_HANDLE) {
            vkDestroyDescriptorSetLayout(device, setLayoutPerPass, nullptr);
            setLayoutPerPass = VK_NULL_HANDLE;
        }
        if (samplerPointClamp != VK_NULL_HANDLE) {
            vkDestroySampler(device, samplerPointClamp, nullptr);
            samplerPointClamp = VK_NULL_HANDLE;
        }
        quad.End();
        for (uint32_t i = 0; i < kMaxSwapchainImages; ++i) {
            if (renderFinishedSemaphores[i] != VK_NULL_HANDLE) {
                vkDestroySemaphore(device, renderFinishedSemaphores[i], nullptr);
                renderFinishedSemaphores[i] = VK_NULL_HANDLE;
            }
        }
        for (uint32_t i = 0; i < Constants::FramesInFlight; ++i) {
            if (inFlightFences[i] != VK_NULL_HANDLE) {
                vkDestroyFence(device, inFlightFences[i], nullptr);
                inFlightFences[i] = VK_NULL_HANDLE;
            }
            if (imageAvailableSemaphores[i] != VK_NULL_HANDLE) {
                vkDestroySemaphore(device, imageAvailableSemaphores[i], nullptr);
                imageAvailableSemaphores[i] = VK_NULL_HANDLE;
            }
        }
        for (uint32_t i = 0; i < Constants::FramesInFlight; ++i) {
            if (commandPools[i] != VK_NULL_HANDLE) {
                vkDestroyCommandPool(device, commandPools[i], nullptr);  // frees commandBuffers[i] too
                commandPools[i] = VK_NULL_HANDLE;
            }
            commandBuffers[i] = VK_NULL_HANDLE;
        }
        frameIndex = 0;

        if (uploadCommandPool != VK_NULL_HANDLE) {
            vkDestroyCommandPool(device, uploadCommandPool, nullptr);
            uploadCommandPool = VK_NULL_HANDLE;
        }

        for (uint32_t i = 0; i < kMaxSwapchainImages; ++i) {
            if (swapchainFramebuffers[i] != VK_NULL_HANDLE) {
                vkDestroyFramebuffer(device, swapchainFramebuffers[i], nullptr);
                swapchainFramebuffers[i] = VK_NULL_HANDLE;
            }
        }
        if (renderPass != VK_NULL_HANDLE) {
            vkDestroyRenderPass(device, renderPass, nullptr);
            renderPass = VK_NULL_HANDLE;
        }
        for (uint32_t i = 0; i < kMaxSwapchainImages; ++i) {
            if (swapchainImageViews[i] != VK_NULL_HANDLE) {
                vkDestroyImageView(device, swapchainImageViews[i], nullptr);
                swapchainImageViews[i] = VK_NULL_HANDLE;
            }
        }
        if (swapchain != VK_NULL_HANDLE) {
            vkDestroySwapchainKHR(device, swapchain, nullptr);
            swapchain = VK_NULL_HANDLE;
        }
        vkDestroyDevice(device, nullptr);
        device = VK_NULL_HANDLE;
    }
    graphicsQueue = VK_NULL_HANDLE;
    swapchainFormat = VK_FORMAT_UNDEFINED;
    swapchainExtent = {};
    swapchainImageCount = 0;  // the images were owned by the swapchain

    if (surface != VK_NULL_HANDLE) {
        vkDestroySurfaceKHR(instance, surface, nullptr);
        surface = VK_NULL_HANDLE;
    }

    if (debugMessenger != VK_NULL_HANDLE) {
        auto destroyMessenger = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
            vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT"));
        if (destroyMessenger != nullptr) {
            destroyMessenger(instance, debugMessenger, nullptr);
        }
        debugMessenger = VK_NULL_HANDLE;
    }

    if (instance != VK_NULL_HANDLE) {
        vkDestroyInstance(instance, nullptr);
        instance = VK_NULL_HANDLE;
    }

    physicalDevice = VK_NULL_HANDLE;
    graphicsQueueFamily = UINT32_MAX;

    ME::Log("RendererVulkan::End");
}

void ME::RendererVulkan::OnPause() {}

void ME::RendererVulkan::SetVsyncEnabled(bool /*enabled*/) {}

bool ME::RendererVulkan::CreateInstance() {
#ifndef NDEBUG
    validationEnabled = HasValidationLayer();
    if (!validationEnabled) {
        ME::LogWarning("RendererVulkan: validation layer not found; running without it");
    }
#endif

    const char* extensions[kMaxInstanceExtensions] = {};
    uint32_t extensionCount = 0;

    // View specific extensions. Different for win and android.
    const uint32_t viewExtensionCount = view->GetInstanceExtensionCount();
    const char* const* viewExtensions = view->GetInstanceExtensions();
    for (uint32_t i = 0; i < viewExtensionCount; ++i) {
        if (extensionCount == kMaxInstanceExtensions) {
            ME::LogError("RendererVulkan: too many instance extensions");
            return false;
        }
        extensions[extensionCount++] = viewExtensions[i];
    }
    if (validationEnabled) {
        if (extensionCount == kMaxInstanceExtensions) {
            ME::LogError("RendererVulkan: too many instance extensions");
            return false;
        }
        extensions[extensionCount++] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
    }

    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "MokshaGame";
    appInfo.pEngineName = "Moksha";
    appInfo.engineVersion = VK_MAKE_API_VERSION(0, 0, 1, 0);
    appInfo.apiVersion = VK_API_VERSION_1_1;

    VkInstanceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
    createInfo.enabledExtensionCount = extensionCount;
    createInfo.ppEnabledExtensionNames = extensions;

    VkDebugUtilsMessengerCreateInfoEXT messengerInfo = MakeMessengerCreateInfo();

    if (validationEnabled) {
        createInfo.enabledLayerCount = 1;
        createInfo.ppEnabledLayerNames = &kValidationLayer;
        createInfo.pNext = &messengerInfo;
    }

    const VkResult result = vkCreateInstance(&createInfo, nullptr, &instance);
    if (result < 0) {
        ME::LogError("RendererVulkan: vkCreateInstance failed (", static_cast<int>(result), ")");
        return false;
    }
    ME::Log("RendererVulkan: instance created (Vulkan 1.1", validationEnabled ? ", validation on)" : ")");

    return true;
}

bool ME::RendererVulkan::SetupDebugMessenger() {
    if (!validationEnabled) {
        return true;  // release build, or no layer: nothing to set up
    }

    // Extension functions aren't exported by the loader; ask the instance for them.
    auto createMessenger = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
        vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT"));
    if (createMessenger == nullptr) {
        ME::LogWarning("RendererVulkan: vkCreateDebugUtilsMessengerEXT not found; no validation messages");
        return true;
    }

    const VkDebugUtilsMessengerCreateInfoEXT info = MakeMessengerCreateInfo();
    const VkResult result = createMessenger(instance, &info, nullptr, &debugMessenger);
    if (result < 0) {
        ME::LogWarning("RendererVulkan: debug messenger failed (", static_cast<int>(result),
                       "); no validation messages");
        return true;
    }
    ME::Log("RendererVulkan: debug messenger on");
    return true;
}

bool ME::RendererVulkan::PickPhysicalDevice() {
    VkPhysicalDevice gpus[kMaxPhysicalDevices] = {};
    uint32_t gpuCount = kMaxPhysicalDevices;
    const VkResult result = vkEnumeratePhysicalDevices(instance, &gpuCount, gpus);
    if (result < 0 || gpuCount == 0) {
        ME::LogError("RendererVulkan: no GPU with Vulkan found (", static_cast<int>(result), ")");
        return false;
    }

    bool pickedIsDiscrete = false;
    for (uint32_t i = 0; i < gpuCount; ++i) {
        VkPhysicalDeviceProperties props = {};
        vkGetPhysicalDeviceProperties(gpus[i], &props);
        ME::Log("RendererVulkan: GPU ", i, ": ", props.deviceName, " (",
                ME::VulkanUtils::DeviceTypeName(props.deviceType), ", Vulkan ", VK_API_VERSION_MAJOR(props.apiVersion),
                ".", VK_API_VERSION_MINOR(props.apiVersion), ".", VK_API_VERSION_PATCH(props.apiVersion), ")");

        if (props.apiVersion < VK_API_VERSION_1_1) {
            continue;  // too old for us
        }
        const bool isDiscrete = props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
        if (physicalDevice == VK_NULL_HANDLE || (isDiscrete && !pickedIsDiscrete)) {
            physicalDevice = gpus[i];
            pickedIsDiscrete = isDiscrete;
        }
    }

    if (physicalDevice == VK_NULL_HANDLE) {
        ME::LogError("RendererVulkan: no GPU supports Vulkan 1.1");
        return false;
    }
    VkPhysicalDeviceProperties picked = {};
    vkGetPhysicalDeviceProperties(physicalDevice, &picked);
    ME::Log("RendererVulkan: using ", picked.deviceName);

    return true;
}

bool ME::RendererVulkan::CreateSurface() {
    if (!view->CreateSurface(instance, &surface)) {
        return false;
    }
    ME::Log("RendererVulkan: surface created");
    return true;
}

bool ME::RendererVulkan::PickQueueFamily() {
    VkQueueFamilyProperties families[kMaxQueueFamilies] = {};
    uint32_t familyCount = kMaxQueueFamilies;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &familyCount, families);

    for (uint32_t i = 0; i < familyCount; ++i) {
        const VkQueueFamilyProperties& family = families[i];
        VkBool32 canPresent = VK_FALSE;
        if (vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface, &canPresent) < 0) {
            ME::LogWarning("RendererVulkan: present query failed for queue family ", i);
            continue;
        }

        if ((family.queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0 && canPresent == VK_TRUE) {
            graphicsQueueFamily = i;
            ME::Log("RendererVulkan: using queue family ", i, " (", family.queueCount, " queues)");
            return true;
        }
    }

    ME::LogError("RendererVulkan: no queue family supports graphics and present");
    return false;
}

bool ME::RendererVulkan::CreateLogicalDevice() {
    const float priority = 1.0f;
    VkDeviceQueueCreateInfo queueInfo = {};
    queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueInfo.queueFamilyIndex = graphicsQueueFamily;
    queueInfo.queueCount = 1;
    queueInfo.pQueuePriorities = &priority;

    const char* const extensions[] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

    VkDeviceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.queueCreateInfoCount = 1;
    createInfo.pQueueCreateInfos = &queueInfo;
    createInfo.enabledExtensionCount = 1;
    createInfo.ppEnabledExtensionNames = extensions;
    createInfo.pEnabledFeatures = nullptr;

    const VkResult result = vkCreateDevice(physicalDevice, &createInfo, nullptr, &device);

    if (result < 0) {
        ME::LogError("RendererVulkan: vkCreateDevice failed (", static_cast<int>(result), ")");
        return false;
    }

    vkGetDeviceQueue(device, graphicsQueueFamily, 0, &graphicsQueue);
    ME::Log("RendererVulkan: device created");
    return true;
}

bool ME::RendererVulkan::CreateSwapchain() {
    VkSurfaceCapabilitiesKHR caps = {};
    VkResult result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &caps);
    if (result < 0) {
        ME::LogError("RendererVulkan: vkGetPhysicalDeviceSurfaceCapabilitiesKHR failed (", static_cast<int>(result),
                     ")");
        return false;
    }

    // The surface dictates the size, unless it reports UINT32_MAX ("app chooses"): then the window's pixel size.
    if (caps.currentExtent.width != UINT32_MAX) {
        swapchainExtent = caps.currentExtent;
    } else {
        swapchainExtent.width =
            ME::Math::Clamp(view->GetPixelWidth(), caps.minImageExtent.width, caps.maxImageExtent.width);
        swapchainExtent.height =
            ME::Math::Clamp(view->GetPixelHeight(), caps.minImageExtent.height, caps.maxImageExtent.height);
    }

    VkSurfaceFormatKHR surfaceFormats[kMaxSurfaceFormats] = {};
    uint32_t formatCount = kMaxSurfaceFormats;
    result = vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, surfaceFormats);
    if (result < 0) {
        ME::LogError("RendererVulkan: vkGetPhysicalDeviceSurfaceFormatsKHR failed (", static_cast<int>(result), ")");
        return false;
    }

    // An 8-bit sRGB format; Windows drivers usually offer BGRA, Android ones RGBA.
    swapchainFormat = VK_FORMAT_UNDEFINED;
    for (uint32_t i = 0; i < formatCount; ++i) {
        const VkFormat format = surfaceFormats[i].format;
        if ((format == VK_FORMAT_R8G8B8A8_SRGB || format == VK_FORMAT_B8G8R8A8_SRGB) &&
            surfaceFormats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            swapchainFormat = format;
            break;
        }
    }

    if (swapchainFormat == VK_FORMAT_UNDEFINED) {
        ME::LogError("RendererVulkan: no suitable surface format found");
        return false;
    }

    uint32_t imageCount = caps.minImageCount + 1;
    if (caps.maxImageCount != 0 && imageCount > caps.maxImageCount) {
        imageCount = caps.maxImageCount;
    }

    VkSwapchainCreateInfoKHR info = {};
    info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    info.surface = surface;
    info.minImageCount = imageCount;
    info.imageFormat = swapchainFormat;
    info.imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;  // the only one the format search accepts
    info.imageExtent = swapchainExtent;
    info.imageArrayLayers = 1;
    info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    info.preTransform = caps.currentTransform;
    // Many Android surfaces offer only INHERIT.
    info.compositeAlpha = (caps.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR) != 0
                              ? VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR
                              : VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR;
    info.presentMode = VK_PRESENT_MODE_FIFO_KHR;
    info.clipped = VK_TRUE;
    info.oldSwapchain = VK_NULL_HANDLE;

    result = vkCreateSwapchainKHR(device, &info, nullptr, &swapchain);
    if (result < 0) {
        ME::LogError("RendererVulkan: vkCreateSwapchainKHR failed (", static_cast<int>(result), ")");
        return false;
    }

    // The driver may make more images than asked; VK_INCOMPLETE means the array is too small.
    swapchainImageCount = kMaxSwapchainImages;
    result = vkGetSwapchainImagesKHR(device, swapchain, &swapchainImageCount, swapchainImages);
    if (result != VK_SUCCESS) {
        ME::LogError("RendererVulkan: vkGetSwapchainImagesKHR failed (", static_cast<int>(result), ")");
        return false;
    }

    ME::Log("RendererVulkan: swapchain ", swapchainExtent.width, "x", swapchainExtent.height, ", ", swapchainImageCount,
            " images, format ", static_cast<int>(swapchainFormat));
    return true;
}

bool ME::RendererVulkan::CreateSwapchainImageViews() {
    for (uint32_t i = 0; i < swapchainImageCount; ++i) {
        VkImageViewCreateInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        info.image = swapchainImages[i];
        info.viewType = VK_IMAGE_VIEW_TYPE_2D;
        info.format = swapchainFormat;
        info.components = {};
        info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        info.subresourceRange.baseMipLevel = 0;
        info.subresourceRange.levelCount = 1;
        info.subresourceRange.baseArrayLayer = 0;
        info.subresourceRange.layerCount = 1;

        const VkResult result = vkCreateImageView(device, &info, nullptr, &swapchainImageViews[i]);
        if (result < 0) {
            ME::LogError("RendererVulkan: vkCreateImageView failed for image ", i, " (", static_cast<int>(result), ")");
            return false;
        }
    }

    ME::Log("RendererVulkan: ", swapchainImageCount, " image views created");
    return true;
}

bool ME::RendererVulkan::CreateRenderPass() {
    VkAttachmentDescription color = {};
    color.format = swapchainFormat;
    color.samples = VK_SAMPLE_COUNT_1_BIT;
    color.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    color.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    color.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    color.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    color.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    color.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorRef = {};
    colorRef.attachment = 0;
    colorRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorRef;

    VkSubpassDependency dependency = {};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    info.attachmentCount = 1;
    info.pAttachments = &color;
    info.subpassCount = 1;
    info.pSubpasses = &subpass;
    info.dependencyCount = 1;
    info.pDependencies = &dependency;

    const VkResult result = vkCreateRenderPass(device, &info, nullptr, &renderPass);
    if (result < 0) {
        ME::LogError("RendererVulkan: vkCreateRenderPass failed (", static_cast<int>(result), ")");
        return false;
    }

    ME::Log("RendererVulkan: render pass created");
    return true;
}

bool ME::RendererVulkan::CreateSwapchainFramebuffers() {
    for (uint32_t i = 0; i < swapchainImageCount; ++i) {
        VkFramebufferCreateInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        info.renderPass = renderPass;
        info.attachmentCount = 1;
        info.pAttachments = &swapchainImageViews[i];
        info.width = swapchainExtent.width;
        info.height = swapchainExtent.height;
        info.layers = 1;

        const VkResult result = vkCreateFramebuffer(device, &info, nullptr, &swapchainFramebuffers[i]);
        if (result < 0) {
            ME::LogError("RendererVulkan: vkCreateFramebuffer failed for image ", i, " (", static_cast<int>(result),
                         ")");
            return false;
        }
    }

    ME::Log("RendererVulkan: ", swapchainImageCount, " framebuffers created");
    return true;
}

bool ME::RendererVulkan::CreateCommandBuffers() {
    for (uint32_t i = 0; i < Constants::FramesInFlight; ++i) {
        VkCommandPoolCreateInfo poolInfo = {};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.queueFamilyIndex = graphicsQueueFamily;
        // Re-recorded every frame; Draw resets the whole pool (vkResetCommandPool), like a DX12 allocator.
        poolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;

        const VkResult poolResult = vkCreateCommandPool(device, &poolInfo, nullptr, &commandPools[i]);
        if (poolResult < 0) {
            ME::LogError("RendererVulkan: vkCreateCommandPool failed for frame ", i, " (", static_cast<int>(poolResult),
                         ")");
            return false;
        }

        VkCommandBufferAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = commandPools[i];
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = 1;

        const VkResult allocResult = vkAllocateCommandBuffers(device, &allocInfo, &commandBuffers[i]);
        if (allocResult < 0) {
            ME::LogError("RendererVulkan: vkAllocateCommandBuffers failed for frame ", i, " (",
                         static_cast<int>(allocResult), ")");
            return false;
        }
    }

    VkCommandPoolCreateInfo uploadPoolInfo = {};
    uploadPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    uploadPoolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
    uploadPoolInfo.queueFamilyIndex = graphicsQueueFamily;
    VkResult uploadPoolResult = vkCreateCommandPool(device, &uploadPoolInfo, nullptr, &uploadCommandPool);
    if (uploadPoolResult != VK_SUCCESS) {
        ME::LogError("RendererVulkan: vkCreateCommandPool failed for upload pool (", static_cast<int>(uploadPoolResult),
                     ")");
        return false;
    }

    ME::Log("RendererVulkan: ", Constants::FramesInFlight, " command pools and buffers created");
    return true;
}

bool ME::RendererVulkan::CreateSyncObjects() {
    VkSemaphoreCreateInfo semaphoreInfo = {};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo = {};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;  // the first wait on each slot returns at once

    // Per frame slot: acquire signals imageAvailable; the slot's submit signals its fence.
    for (uint32_t i = 0; i < Constants::FramesInFlight; ++i) {
        VkResult result = vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]);
        if (result < 0) {
            ME::LogError("RendererVulkan: vkCreateSemaphore (image available) failed for frame ", i, " (",
                         static_cast<int>(result), ")");
            return false;
        }
        result = vkCreateFence(device, &fenceInfo, nullptr, &inFlightFences[i]);
        if (result < 0) {
            ME::LogError("RendererVulkan: vkCreateFence failed for frame ", i, " (", static_cast<int>(result), ")");
            return false;
        }
    }

    // Per swapchain image: present waits on it and signals no fence, so the image's next acquire proves it is free.
    for (uint32_t i = 0; i < swapchainImageCount; ++i) {
        const VkResult result = vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]);
        if (result < 0) {
            ME::LogError("RendererVulkan: vkCreateSemaphore (render finished) failed for image ", i, " (",
                         static_cast<int>(result), ")");
            return false;
        }
    }

    ME::Log("RendererVulkan: sync objects created (", Constants::FramesInFlight, " frames, ", swapchainImageCount,
            " images)");
    return true;
}

bool ME::RendererVulkan::CreatePSO() {
    setLayoutPerPass = PipelineLayoutVulkan::CreateSetLayoutPerPass(device);
    if (setLayoutPerPass == VK_NULL_HANDLE) {
        ME::LogError("RendererVulkan: failed to create the per-pass descriptor set layout");
        return false;
    }

    setLayoutPerSprite = PipelineLayoutVulkan::CreateSetLayoutPerSprite(device);
    if (setLayoutPerSprite == VK_NULL_HANDLE) {
        ME::LogError("RendererVulkan: failed to create the per-sprite descriptor set layout");
        return false;
    }

    setLayoutTexture = PipelineLayoutVulkan::CreateSetLayoutTexture(device, samplerPointClamp);
    if (setLayoutTexture == VK_NULL_HANDLE) {
        ME::LogError("RendererVulkan: failed to create the texture descriptor set layout");
        return false;
    }

    layout2DAtl =
        PipelineLayoutVulkan::CreateLayout2DAtlas(device, setLayoutPerPass, setLayoutPerSprite, setLayoutTexture);
    if (layout2DAtl == VK_NULL_HANDLE) {
        ME::LogError("RendererVulkan: failed to create the 2D atlas pipeline layout");
        return false;
    }

    pso2DAtl = PSOVulkan::CreatePSO2DAtlas(device, renderPass, layout2DAtl, "sprite_atlas");
    if (pso2DAtl == VK_NULL_HANDLE) {
        ME::LogError("RendererVulkan: failed to create the 2D atlas pipeline");
        return false;
    }

    return true;
}

bool ME::RendererVulkan::CreateDescriptorSets() {
    if (!perPassBuffer.Init(physicalDevice, device, 1, sizeof(ME::CBPerPass))) {
        ME::LogError("RendererVulkan: failed to create the per-pass constant buffer");
        return false;
    }
    if (!perSpriteBuffer.Init(physicalDevice, device, 1, sizeof(ME::CBPerSprite))) {
        ME::LogError("RendererVulkan: failed to create the per-sprite constant buffer");
        return false;
    }

    VkDescriptorPoolSize poolSizes[2] = {};
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    poolSizes[0].descriptorCount = 2;
    poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[1].descriptorCount = 1;

    VkDescriptorPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.maxSets = 3;
    poolInfo.poolSizeCount = 2;
    poolInfo.pPoolSizes = poolSizes;
    VkResult result = vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptorPool);
    if (result != VK_SUCCESS) {
        ME::LogError("RendererVulkan: vkCreateDescriptorPool failed (", static_cast<int>(result), ")");
        return false;
    }

    VkDescriptorSetAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = descriptorPool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &setLayoutPerPass;
    result = vkAllocateDescriptorSets(device, &allocInfo, &perPassSet);
    if (result != VK_SUCCESS) {
        ME::LogError("RendererVulkan: vkAllocateDescriptorSets (per pass) failed (", static_cast<int>(result), ")");
        return false;
    }

    allocInfo.pSetLayouts = &setLayoutPerSprite;
    result = vkAllocateDescriptorSets(device, &allocInfo, &perSpriteSet);
    if (result != VK_SUCCESS) {
        ME::LogError("RendererVulkan: vkAllocateDescriptorSets (per sprite) failed (", static_cast<int>(result), ")");
        return false;
    }

    allocInfo.pSetLayouts = &setLayoutTexture;
    result = vkAllocateDescriptorSets(device, &allocInfo, &textureSet);
    if (result != VK_SUCCESS) {
        ME::LogError("RendererVulkan: vkAllocateDescriptorSets (texture) failed (", static_cast<int>(result), ")");
        return false;
    }

    VkDescriptorImageInfo imageInfo = {};
    imageInfo.sampler = VK_NULL_HANDLE;  // ignored: the layout's immutable sampler is used
    imageInfo.imageView = testTexture.GetImageView();
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    VkDescriptorBufferInfo passInfo = {};
    passInfo.buffer = perPassBuffer.GetBuffer();
    passInfo.offset = 0;
    passInfo.range = perPassBuffer.GetElementSize();

    VkDescriptorBufferInfo spriteInfo = {};
    spriteInfo.buffer = perSpriteBuffer.GetBuffer();
    spriteInfo.offset = 0;
    spriteInfo.range = perSpriteBuffer.GetElementSize();

    VkWriteDescriptorSet writes[3] = {};
    writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writes[0].dstSet = perPassSet;
    writes[0].dstBinding = 0;
    writes[0].dstArrayElement = 0;
    writes[0].descriptorCount = 1;
    writes[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    writes[0].pBufferInfo = &passInfo;
    writes[1] = writes[0];
    writes[1].dstSet = perSpriteSet;
    writes[1].pBufferInfo = &spriteInfo;
    writes[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writes[2].dstSet = textureSet;
    writes[2].dstBinding = 0;
    writes[2].dstArrayElement = 0;
    writes[2].descriptorCount = 1;
    writes[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    writes[2].pImageInfo = &imageInfo;

    // Written once: the descriptors cover every frame's copy, and each frame picks its copy with a dynamic offset.
    vkUpdateDescriptorSets(device, 3, writes, 0, nullptr);

    ME::Log("RendererVulkan: descriptor sets created");
    return true;
}

#endif  // ME_VULKAN
