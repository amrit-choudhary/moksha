#ifdef ME_VULKAN

#include "renderer_vulkan.h"

#include <cstring>

#include "moksha/logging/logging.h"
#include "vulkan_view.h"

namespace {

const char* const kValidationLayer = "VK_LAYER_KHRONOS_validation";
constexpr uint32_t kMaxInstanceExtensions = 8;

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

constexpr uint32_t kMaxPhysicalDevices = 8;

// Readable name of a GPU type, for the log.
const char* DeviceTypeName(VkPhysicalDeviceType type) {
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

    if (!PickPhysicalDevice()) {
        ME::LogError("RendererVulkan: failed to pick a physical device");
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

void ME::RendererVulkan::Draw() {}

void ME::RendererVulkan::End() {
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
        ME::Log("RendererVulkan: GPU ", i, ": ", props.deviceName, " (", DeviceTypeName(props.deviceType), ", Vulkan ",
                VK_API_VERSION_MAJOR(props.apiVersion), ".", VK_API_VERSION_MINOR(props.apiVersion), ".",
                VK_API_VERSION_PATCH(props.apiVersion), ")");

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

#endif  // ME_VULKAN
