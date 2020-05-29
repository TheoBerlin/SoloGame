#include "DeviceVK.hpp"

#include <Engine/Rendering/Window.hpp>

#include <vulkan/vulkan_win32.h>

DeviceVK::DeviceVK(const DeviceInfoVK& deviceInfo)
    :Device(deviceInfo.pBackBuffer, deviceInfo.pDepthTexture),
    m_Instance(deviceInfo.Instance),
    m_PhysicalDevice(deviceInfo.PhysicalDevice),
    m_Device(deviceInfo.Device),
    m_Surface(deviceInfo.Surface),
    m_SwapchainImages(deviceInfo.SwapchainImages),
    m_SwapchainImageViews(deviceInfo.SwapchainImageViews),
    m_SwapchainFormat(deviceInfo.SwapchainFormat),
    m_DebugMessenger(deviceInfo.DebugMessenger),
    m_QueueFamilyIndices(deviceInfo.QueueFamilyIndices)
{}

DeviceVK::~DeviceVK()
{
    #ifdef _DEBUG
        auto destroyDebugUtilsMessengerEXT = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(m_Instance, "vkDestroyDebugUtilsMessengerEXT");
        if (destroyDebugUtilsMessengerEXT) {
            destroyDebugUtilsMessengerEXT(m_Instance, m_DebugMessenger, nullptr);
        } else {
            LOG_ERROR("Failed to retrieve function pointer to vkDestroyDebugUtilsMessengerEXT");
        }
    #endif

    for (VkImageView imageView : m_SwapchainImageViews) {
        vkDestroyImageView(m_Device, imageView, nullptr);
    }

    vkDestroySwapchainKHR(m_Device, m_Swapchain, nullptr);
    vkDestroyDevice(m_Device, nullptr);
    vkDestroySurfaceKHR(m_Instance, m_Surface, nullptr);
    vkDestroyInstance(m_Instance, nullptr);
}

VKAPI_ATTR VkBool32 VKAPI_CALL DeviceVK::vulkanCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
{
    switch (messageSeverity) {
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
            LOG_INFO("Validation layer: %s", pCallbackData->pMessage);
            return VK_FALSE;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
            LOG_WARNING("Validation layer: %s", pCallbackData->pMessage);
            return VK_FALSE;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
        default:
            LOG_ERROR("Validation layer: %s", pCallbackData->pMessage);
            return VK_FALSE;
    }
}
