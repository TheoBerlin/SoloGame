#include "DeviceCreatorVK.hpp"

#include <Engine/Rendering/Window.hpp>
#include <Engine/Utils/Debug.hpp>

#include <vulkan/vulkan_win32.h>

const std::vector<const char*> g_RequiredLayerNames         = { "VK_LAYER_KHRONOS_validation" };
const std::vector<const char*> g_RequiredDeviceExtensions   = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

Device* DeviceCreatorVK::createDevice(const SwapchainInfo& swapChainInfo, const Window* pWindow)
{
    bool debugMode = false;
    #ifdef _DEBUG
        debugMode = true;
    #endif

    if (!initInstance(pWindow, debugMode)) {
        return nullptr;
    }

    if (debugMode && !initDebugLayerCallback()) {
        return nullptr;
    }

    if (!initSurface(pWindow)) {
        return nullptr;
    }

    if (!pickPhysicalDevice()) {
        return nullptr;
    }

    if (!pickQueueFamilyIndices()) {
        return nullptr;
    }

    if (!initLogicalDevice()) {
        return nullptr;
    }

    if (!initSwapchain(pWindow)) {
        return nullptr;
    }

    if (!initSwapchainImageViews()) {
        return nullptr;
    }

    DeviceInfoVK deviceInfo = {};
    deviceInfo.Instance             = m_Instance;
    deviceInfo.PhysicalDevice       = m_PhysicalDevice;
    deviceInfo.Device               = m_Device;
    deviceInfo.Surface              = m_Surface;
    deviceInfo.Swapchain            = m_Swapchain;
    deviceInfo.SwapchainImages      = m_SwapchainImages;
    deviceInfo.SwapchainImageViews  = m_SwapchainImageViews;
    deviceInfo.SwapchainFormat      = m_SwapchainFormat;
    deviceInfo.DebugMessenger       = m_DebugMessenger;
    deviceInfo.QueueFamilyIndices   = m_QueueFamilyIndices;

    return DBG_NEW DeviceVK(deviceInfo);
}

bool DeviceCreatorVK::initInstance(const Window* pWindow, bool debugMode)
{
    VkApplicationInfo appInfo   = {};
    appInfo.sType               = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName    = "SoloGame";
    appInfo.applicationVersion  = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName         = "SoloEngine";
    appInfo.engineVersion       = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion          = VK_MAKE_VERSION(1, 2, 0);

    std::vector<std::string> requiredExtensions = pWindow->getRequiredInstanceExtensions();
    requiredExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    if (!verifyRequiredExtensionsSupported(requiredExtensions)) {
        return false;
    }

    // Convert strings to char*
    std::vector<const char*> requiredExtensionsChars;
    requiredExtensionsChars.reserve(requiredExtensions.size());
    for (const std::string& requiredExtension : requiredExtensions) {
        requiredExtensionsChars.push_back(requiredExtension.c_str());
    }

    VkInstanceCreateInfo instanceInfo = {};
    instanceInfo.sType                      = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceInfo.pApplicationInfo           = &appInfo;
    instanceInfo.ppEnabledExtensionNames    = requiredExtensionsChars.data();
    instanceInfo.enabledExtensionCount      = (uint32_t)requiredExtensionsChars.size();

    if (debugMode) {
        if (!verifyLayerSupport()) {
            return false;
        }

        instanceInfo.ppEnabledLayerNames    = g_RequiredLayerNames.data();
        instanceInfo.enabledLayerCount      = (uint32_t)g_RequiredLayerNames.size();
    }

    if (vkCreateInstance(&instanceInfo, nullptr, &m_Instance) != VK_SUCCESS) {
        LOG_ERROR("Failed to create vulkan instance");
        return false;
    }

    return true;
}

bool DeviceCreatorVK::initDebugLayerCallback()
{
    VkDebugUtilsMessengerCreateInfoEXT debugCallbackInfo = {};
    debugCallbackInfo.sType             = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    debugCallbackInfo.messageSeverity   = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    debugCallbackInfo.messageType       = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    debugCallbackInfo.pfnUserCallback   = DeviceVK::vulkanCallback;

    // Get pointer to function vkCreateDebugUtilsMessengerEXT
    auto createDebugUtilsMessengerEXT = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(m_Instance, "vkCreateDebugUtilsMessengerEXT");
    if (!createDebugUtilsMessengerEXT) {
        LOG_ERROR("Failed to retrieve function pointer to vkCreateDebugUtilsMessengerEXT");
        return false;
    } else if (createDebugUtilsMessengerEXT(m_Instance, &debugCallbackInfo, nullptr, &m_DebugMessenger) != VK_SUCCESS) {
        LOG_ERROR("Failed to create debug messenger");
        return false;
    }

    return true;
}

bool DeviceCreatorVK::initSurface(const Window* pWindow)
{
    VkWin32SurfaceCreateInfoKHR surfaceInfo = {};
    surfaceInfo.sType       = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    surfaceInfo.hwnd        = pWindow->getHWND();
    surfaceInfo.hinstance   = GetModuleHandle(nullptr);

    return vkCreateWin32SurfaceKHR(m_Instance, &surfaceInfo, nullptr, &m_Surface) == VK_SUCCESS;
}

bool DeviceCreatorVK::pickPhysicalDevice()
{
    uint32_t deviceCount = 0u;
    if (vkEnumeratePhysicalDevices(m_Instance, &deviceCount, nullptr) != VK_SUCCESS) {
        LOG_ERROR("Failed to retrieve physical device count");
        return false;
    }

    std::vector<VkPhysicalDevice> physicalDevices(deviceCount);
    if (vkEnumeratePhysicalDevices(m_Instance, &deviceCount, physicalDevices.data()) != VK_SUCCESS) {
        LOG_ERROR("Failed to retrieve physical devices");
        return false;
    }

    // Rate devices based on their properties, to decide which device to use
    std::vector<uint32_t> deviceRatings(deviceCount);
    uint32_t highestRating = 0u, highestRatingIdx = 0u;

    for (size_t deviceIdx = 0u; deviceIdx < deviceCount; deviceIdx++) {
        VkPhysicalDevice device = physicalDevices[deviceIdx];
        uint32_t& deviceRating  = deviceRatings[deviceIdx];
        deviceRating = 0u;

        VkPhysicalDeviceProperties deviceProperties = {};
        VkPhysicalDeviceFeatures deviceFeatures     = {};

        vkGetPhysicalDeviceProperties(device, &deviceProperties);
        vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

        deviceRating += VK_VERSION_MAJOR(deviceProperties.apiVersion) * 100u + VK_VERSION_MINOR(deviceProperties.apiVersion) * 10u;
        deviceRating += uint32_t(deviceFeatures.geometryShader) * 100u;
        deviceRating += (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) * 1000u;

        if (!verifyRequiredDeviceExtensionsSupported(device)) {
            deviceRating = 0u;
        } else {
            SwapchainSupportDetails swapChainSupportDetails = {};
            if (!querySwapchainSupport(device, swapChainSupportDetails)) {
                return false;
            }

            if (swapChainSupportDetails.PresentModes.empty() || swapChainSupportDetails.Formats.empty()) {
                deviceRating = 0u;
            }
        }

        if (deviceRating > highestRating) {
            highestRating       = deviceRating;
            highestRatingIdx    = highestRatingIdx;
        }
    }

    if (highestRating == 0u) {
        LOG_ERROR("No suitable device found");
        return false;
    }

    m_PhysicalDevice = physicalDevices[highestRatingIdx];
    return true;
}

bool DeviceCreatorVK::pickQueueFamilyIndices()
{
    uint32_t queueFamilyCount = 0u;
    vkGetPhysicalDeviceQueueFamilyProperties(m_PhysicalDevice, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilyProperties(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(m_PhysicalDevice, &queueFamilyCount, queueFamilyProperties.data());

    // Sort queue families by their queue counts, by mapping queue counts to family indices (descending order)
    std::map<uint32_t, uint32_t, std::greater<uint32_t>> sortedQueueFamilies;

    // .. and simultaneously look for a present queue family
    bool foundPresentFamily = false;

    for (size_t queueFamilyIdx = 0; queueFamilyIdx < queueFamilyProperties.size(); queueFamilyIdx++) {
        const VkQueueFamilyProperties& familyProperties     = queueFamilyProperties[queueFamilyIdx];
        sortedQueueFamilies[familyProperties.queueCount]    = (uint32_t)queueFamilyIdx;

        if (!foundPresentFamily) {
            VkBool32 presentSupported = false;
            if (vkGetPhysicalDeviceSurfaceSupportKHR(m_PhysicalDevice, (uint32_t)queueFamilyIdx, m_Surface, &presentSupported) != VK_SUCCESS) {
                LOG_ERROR("Failed to check for present capabilities, queue family index: %d", queueFamilyIdx);
                return false;
            }

            if (presentSupported) {
                m_QueueFamilyIndices.PresentFamily = (uint32_t)queueFamilyIdx;
                foundPresentFamily = true;
            }
        }
    }

    if (!foundPresentFamily) {
        LOG_ERROR("Failed to find queue family for present operations");
        return false;
    }

    // Choose a queue family for graphics, compute and transfer operations. Queue families with more queues get prioritized.
    std::array<std::pair<VkQueueFlagBits, uint32_t*>, 3> queueFamilyIndices = {
        std::make_pair(VK_QUEUE_GRAPHICS_BIT, &m_QueueFamilyIndices.GraphicsFamily),
        std::make_pair(VK_QUEUE_COMPUTE_BIT, &m_QueueFamilyIndices.ComputeFamily),
        std::make_pair(VK_QUEUE_TRANSFER_BIT, &m_QueueFamilyIndices.TransferFamily)
    };

    std::unordered_set<uint32_t> usedQueueFamilyIndices;

    for (const std::pair<VkQueueFlagBits, uint32_t*>& queueFamilyIndex : queueFamilyIndices) {
        bool pickedFamily = false;

        for (const std::pair<uint32_t, uint32_t>& queueFamily : sortedQueueFamilies) {
            const VkQueueFamilyProperties& familyProperties = queueFamilyProperties[queueFamily.second];
            if (queueFamilyIndex.first == (queueFamilyIndex.first & familyProperties.queueFlags) && !usedQueueFamilyIndices.contains(queueFamily.second)) {
                *queueFamilyIndex.second = queueFamily.second;
                pickedFamily = true;
                usedQueueFamilyIndices.insert(queueFamily.second);
                break;
            }
        }

        if (pickedFamily) {
            continue;
        }

        // No unused queue family was found, use the first one that is compatible with the operations flag
        for (const std::pair<uint32_t, uint32_t>& queueFamily : sortedQueueFamilies) {
            const VkQueueFamilyProperties& familyProperties = queueFamilyProperties[queueFamily.second];
            if (queueFamilyIndex.first == (queueFamilyIndex.first & familyProperties.queueFlags)) {
                *queueFamilyIndex.second = queueFamily.second;
                pickedFamily = true;
                break;
            }
        }

        if (!pickedFamily) {
            LOG_ERROR("Failed to find queue family with flag: %d", (int)queueFamilyIndex.first);
            return false;
        }
    }

    return true;
}

bool DeviceCreatorVK::initLogicalDevice()
{
    std::unordered_set<uint32_t> queueFamilyIndices = {m_QueueFamilyIndices.GraphicsFamily, m_QueueFamilyIndices.TransferFamily, m_QueueFamilyIndices.ComputeFamily, m_QueueFamilyIndices.PresentFamily};
    std::vector<VkDeviceQueueCreateInfo> queueInfos;
    queueInfos.reserve(queueFamilyIndices.size());

    const float queuePriority = 1.0f;

    VkDeviceQueueCreateInfo queueInfo = {};
    queueInfo.sType               = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueInfo.queueCount          = 1;
    queueInfo.pQueuePriorities    = &queuePriority;

    for (uint32_t familyIndex : queueFamilyIndices) {
        queueInfo.queueFamilyIndex = familyIndex;
        queueInfos.push_back(queueInfo);
    }

    VkPhysicalDeviceFeatures deviceFeatures = {};

    VkDeviceCreateInfo deviceInfo = {};
    deviceInfo.sType                    = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceInfo.queueCreateInfoCount     = (uint32_t)queueInfos.size();
    deviceInfo.pQueueCreateInfos        = queueInfos.data();
    deviceInfo.enabledLayerCount        = (uint32_t)g_RequiredLayerNames.size();
    deviceInfo.ppEnabledLayerNames      = g_RequiredLayerNames.data();
    deviceInfo.enabledExtensionCount    = (uint32_t)g_RequiredDeviceExtensions.size();
    deviceInfo.ppEnabledExtensionNames  = g_RequiredDeviceExtensions.data();
    deviceInfo.pEnabledFeatures         = &deviceFeatures;

    if (vkCreateDevice(m_PhysicalDevice, &deviceInfo, nullptr, &m_Device) != VK_SUCCESS) {
        LOG_ERROR("Failed to create device");
        return false;
    }

    return true;
}

bool DeviceCreatorVK::initSwapchain(const Window* pWindow)
{
    SwapchainSupportDetails supportDetails = {};
    if (!querySwapchainSupport(m_PhysicalDevice, supportDetails)) {
        return false;
    }

    uint32_t backbufferCount = std::max(3u, supportDetails.SurfaceCapabilities.minImageCount + 1u);
    if (supportDetails.SurfaceCapabilities.maxImageCount != 0) {
        backbufferCount = std::min(backbufferCount, supportDetails.SurfaceCapabilities.maxImageCount);
    }

    chooseSwapchainFormat(supportDetails.Formats);

    VkSwapchainCreateInfoKHR swapchainInfo = {};
    swapchainInfo.sType             = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchainInfo.surface           = m_Surface;
    swapchainInfo.minImageCount     = backbufferCount;
    swapchainInfo.imageFormat       = m_SwapchainFormat.format;
    swapchainInfo.imageColorSpace   = m_SwapchainFormat.colorSpace;
    swapchainInfo.imageExtent       = chooseSwapchainExtent(supportDetails.SurfaceCapabilities, pWindow);
    swapchainInfo.imageArrayLayers  = 1u;
    swapchainInfo.imageUsage        = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapchainInfo.imageSharingMode  = VK_SHARING_MODE_EXCLUSIVE;
    swapchainInfo.preTransform      = supportDetails.SurfaceCapabilities.currentTransform;
    swapchainInfo.compositeAlpha    = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapchainInfo.presentMode       = chooseSwapchainPresentMode(supportDetails.PresentModes);
    swapchainInfo.clipped           = VK_TRUE;
    swapchainInfo.oldSwapchain      = VK_NULL_HANDLE;

    if (vkCreateSwapchainKHR(m_Device, &swapchainInfo, nullptr, &m_Swapchain) != VK_SUCCESS) {
        LOG_ERROR("Failed to create swapchain");
        return false;
    }

    // Retrieve swapchain images
    uint32_t imageCount = 0u;
    if (vkGetSwapchainImagesKHR(m_Device, m_Swapchain, &imageCount, nullptr) != VK_SUCCESS) {
        LOG_ERROR("Failed to retrieve swapchain image count");
        return false;
    }

    m_SwapchainImages.resize((size_t)imageCount);
    if (vkGetSwapchainImagesKHR(m_Device, m_Swapchain, &imageCount, m_SwapchainImages.data()) != VK_SUCCESS) {
        LOG_ERROR("Failed to retrieve swapchain images");
        return false;
    }

    return true;
}

bool DeviceCreatorVK::initSwapchainImageViews()
{
    m_SwapchainImageViews.resize(m_SwapchainImages.size());

    VkImageViewCreateInfo imageViewInfo = {};
    imageViewInfo.sType     = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    imageViewInfo.viewType  = VK_IMAGE_VIEW_TYPE_2D;
    imageViewInfo.format    = m_SwapchainFormat.format;
    imageViewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    imageViewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    imageViewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    imageViewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
    imageViewInfo.subresourceRange.aspectMask       = VK_IMAGE_ASPECT_COLOR_BIT;
    imageViewInfo.subresourceRange.baseMipLevel     = 0u;
    imageViewInfo.subresourceRange.levelCount       = 1u;
    imageViewInfo.subresourceRange.baseArrayLayer   = 0u;
    imageViewInfo.subresourceRange.layerCount       = 1u;

    for (size_t imageIdx = 0u; imageIdx < m_SwapchainImages.size(); imageIdx += 1u) {
        imageViewInfo.image = m_SwapchainImages[imageIdx];
        if (vkCreateImageView(m_Device, &imageViewInfo, nullptr, &m_SwapchainImageViews[imageIdx]) != VK_SUCCESS) {
            LOG_ERROR("Failed to create swapchain image view");
            return false;
        }
    }

    return true;
}

bool DeviceCreatorVK::verifyRequiredExtensionsSupported(const std::vector<std::string>& extensionNames)
{
    uint32_t availableExtensionCount = 0u;
    if (vkEnumerateInstanceExtensionProperties(nullptr, &availableExtensionCount, nullptr) != VK_SUCCESS) {
        LOG_ERROR("Failed to retrieve available extension count");
        return false;
    }

    std::vector<VkExtensionProperties> availableExtensions(availableExtensionCount);
    if (vkEnumerateInstanceExtensionProperties(nullptr, &availableExtensionCount, availableExtensions.data()) != VK_SUCCESS) {
        LOG_ERROR("Failed to retrieve available extensions");
        return false;
    }

    std::unordered_set<std::string> availableExtensionsSet;
    for (const VkExtensionProperties& availableExtension : availableExtensions) {
        availableExtensionsSet.insert(availableExtension.extensionName);
    }

    bool hasAllExtensions = true;
    for (const std::string& pRequiredExtension : extensionNames) {
        if (!availableExtensionsSet.contains(pRequiredExtension)) {
            LOG_ERROR("Missing required extension: %s", pRequiredExtension);
            hasAllExtensions = false;
        }
    }

    return hasAllExtensions;
}

bool DeviceCreatorVK::verifyRequiredDeviceExtensionsSupported(VkPhysicalDevice physicalDevice)
{
    uint32_t extensionCount = 0u;
    if (vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, nullptr) != VK_SUCCESS) {
        LOG_ERROR("Failed to retrieve supported device extension count");
        return false;
    }

    std::vector<VkExtensionProperties> supportedExtensions(extensionCount);
    if (vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, supportedExtensions.data()) != VK_SUCCESS) {
        LOG_ERROR("Failed to retrieve supported device extensions");
        return false;
    }

    std::unordered_set<std::string> supportedExtensionsSet;
    for (const VkExtensionProperties& extension : supportedExtensions) {
        supportedExtensionsSet.insert(extension.extensionName);
    }

    for (const std::string& requiredExtension : g_RequiredDeviceExtensions) {
        if (!supportedExtensionsSet.contains(requiredExtension)) {
            return false;
        }
    }

    return true;
}

bool DeviceCreatorVK::verifyLayerSupport()
{
    uint32_t availableLayerCount = 0u;
    if (vkEnumerateInstanceLayerProperties(&availableLayerCount, nullptr) != VK_SUCCESS) {
        LOG_ERROR("Failed to retrieve available layer count");
        return false;
    }

    std::vector<VkLayerProperties> availableLayers(availableLayerCount);
    if (vkEnumerateInstanceLayerProperties(&availableLayerCount, availableLayers.data()) != VK_SUCCESS) {
        LOG_ERROR("Failed to retrieve available layers");
        return false;
    }

    std::unordered_set<std::string> availableLayersSet;
    for (const VkLayerProperties& availableLayer : availableLayers) {
        availableLayersSet.insert(availableLayer.layerName);
    }

    bool hasAllLayers = true;
    for (const char* pRequiredLayer : g_RequiredLayerNames) {
        if (!availableLayersSet.contains(pRequiredLayer)) {
            LOG_ERROR("Missing required layer: %s", pRequiredLayer);
            hasAllLayers = false;
        }
    }

    return hasAllLayers;
}

bool DeviceCreatorVK::querySwapchainSupport(VkPhysicalDevice physicalDevice, SwapchainSupportDetails& supportDetails) const
{
    supportDetails = {};

    VkSurfaceCapabilitiesKHR surfaceCapabilities = {};
    if(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, m_Surface, &surfaceCapabilities) != VK_SUCCESS) {
        LOG_ERROR("Failed to get device's surface capabilities");
        return false;
    }

    supportDetails.SurfaceCapabilities = surfaceCapabilities;

    uint32_t formatCount = 0u;
    if (vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, m_Surface, &formatCount, nullptr) != VK_SUCCESS) {
        LOG_ERROR("Failed to get device's supported format count");
        return false;
    }

    supportDetails.Formats.resize((size_t)formatCount);
    if (vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, m_Surface, &formatCount, supportDetails.Formats.data()) != VK_SUCCESS) {
        LOG_ERROR("Failed to get device's supported formats");
        return false;
    }

    uint32_t presentModeCount = 0u;
    if (vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, m_Surface, &presentModeCount, nullptr) != VK_SUCCESS) {
        LOG_ERROR("Failed to get device's supported present mode count");
        return false;
    }

    supportDetails.PresentModes.resize((size_t)presentModeCount);
    if (vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, m_Surface, &presentModeCount, supportDetails.PresentModes.data()) != VK_SUCCESS) {
        LOG_ERROR("Failed to get device's supported present modes");
        return false;
    }

    return true;
}

void DeviceCreatorVK::chooseSwapchainFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
{
    for (const VkSurfaceFormatKHR& format : availableFormats) {
        if (format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR && format.format == VK_FORMAT_B8G8R8A8_SRGB) {
            m_SwapchainFormat = format;
            return;
        }
    }

    m_SwapchainFormat = availableFormats.front();
}

VkPresentModeKHR DeviceCreatorVK::chooseSwapchainPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) const
{
    std::unordered_set<VkPresentModeKHR> availablePresentModesSet(availablePresentModes.begin(), availablePresentModes.end());
    if (availablePresentModesSet.contains(VK_PRESENT_MODE_MAILBOX_KHR)) {
        return VK_PRESENT_MODE_MAILBOX_KHR;
    }

    if (availablePresentModesSet.contains(VK_PRESENT_MODE_IMMEDIATE_KHR)) {
        return VK_PRESENT_MODE_IMMEDIATE_KHR;
    }

    // Guaranteed to be supported
    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D DeviceCreatorVK::chooseSwapchainExtent(const VkSurfaceCapabilitiesKHR& capabilities, const Window* pWindow) const
{
    if (capabilities.currentExtent.width != UINT32_MAX) {
        // Use the window's resolution
        return capabilities.currentExtent;
    }

    // Use the clamped resolution of the window
    VkExtent2D windowExtent = { pWindow->getWidth(), pWindow->getHeight() };
    VkExtent2D swapChainExtent = { 0u, 0u };

    swapChainExtent.width   = std::clamp(windowExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
    swapChainExtent.height  = std::clamp(windowExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

    return swapChainExtent;
}
