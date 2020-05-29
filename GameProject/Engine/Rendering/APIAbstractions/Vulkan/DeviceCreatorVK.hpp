#pragma once

#include <Engine/Rendering/APIAbstractions/DeviceCreator.hpp>
#include <Engine/Rendering/APIAbstractions/Vulkan/DeviceVK.hpp>

class DeviceCreatorVK : public IDeviceCreator
{
public:
    DeviceCreatorVK() = default;
    ~DeviceCreatorVK() = default;

    Device* createDevice(const SwapchainInfo& swapChainInfo, const Window* pWindow) override final;

private:
    bool initInstance(const Window* pWindow, bool debugMode);
    bool initDebugLayerCallback();
    bool initSurface(const Window* pWindow);
    bool pickPhysicalDevice();
    bool pickQueueFamilyIndices();
    bool initLogicalDevice();
    bool initSwapchain(const Window* pWindow);
    bool initSwapchainImageViews();

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
    VkSurfaceKHR m_Surface;
    VkSwapchainKHR m_Swapchain;
    std::vector<VkImage> m_SwapchainImages;
    std::vector<VkImageView> m_SwapchainImageViews;

    VkSurfaceFormatKHR m_SwapchainFormat;

    VkDebugUtilsMessengerEXT m_DebugMessenger;

    QueueFamilyIndices m_QueueFamilyIndices;
};
