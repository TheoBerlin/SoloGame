#pragma once

#include <Engine/Rendering/APIAbstractions/DeviceCreator.hpp>
#include <Engine/Rendering/APIAbstractions/Vulkan/DeviceVK.hpp>

struct SwapchainSupportDetails {
    VkSurfaceCapabilitiesKHR SurfaceCapabilities;
    std::vector<VkSurfaceFormatKHR> Formats;
    std::vector<VkPresentModeKHR> PresentModes;
};

class DeviceCreatorVK : public IDeviceCreator
{
public:
    DeviceCreatorVK() = default;
    ~DeviceCreatorVK() = default;

    Device* createDevice(const SwapchainInfo& swapChainInfo, const Window* pWindow) override final;
    Swapchain* createSwapchain(Device* pDevice) override final;

private:
    bool initInstance(const Window* pWindow, bool debugMode);
    bool initDebugLayerCallback();
    bool initSurface(const Window* pWindow);
    bool pickPhysicalDevice();
    bool pickQueueFamilyIndices();
    bool initLogicalDevice();
    void initQueues();
    bool initAllocator();
    bool initSwapchain(const Window* pWindow);
    bool initSwapchainImageViews();
    bool initBackbuffersAndDepthTextures(DeviceVK* pDevice);

    bool verifyRequiredExtensionsSupported(const std::vector<std::string>& extensionNames);
    bool verifyRequiredDeviceExtensionsSupported(VkPhysicalDevice physicalDevice);
    bool verifyLayerSupport();
    bool querySwapchainSupport(VkPhysicalDevice physicalDevice, SwapchainSupportDetails& supportDetails) const;
    void chooseSwapchainFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
    VkPresentModeKHR chooseSwapchainPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) const;
    VkExtent2D chooseSwapchainExtent(const VkSurfaceCapabilitiesKHR& capabilities, const Window* pWindow) const;

private:
    VkInstance m_Instance;
    VkPhysicalDevice m_PhysicalDevice;
    VkDevice m_Device;
    VmaAllocator m_Allocator;

    glm::uvec2 m_SwapchainResolution;
    Texture* m_ppBackbuffers[MAX_FRAMES_IN_FLIGHT];
    Texture* m_ppDepthTextures[MAX_FRAMES_IN_FLIGHT];

    VkSurfaceKHR m_Surface;
    VkSwapchainKHR m_Swapchain;
    std::vector<VkImage> m_SwapchainImages;
    std::vector<VkImageView> m_SwapchainImageViews;

    VkSurfaceFormatKHR m_SwapchainFormat;

    VkDebugUtilsMessengerEXT m_DebugMessenger;

    QueueFamilyIndices m_QueueFamilyIndices;
    Queues m_Queues;
};
