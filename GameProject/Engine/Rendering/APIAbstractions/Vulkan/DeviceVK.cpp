#include "DeviceVK.hpp"

#include <Engine/Rendering/APIAbstractions/Vulkan/CommandListVK.hpp>
#include <Engine/Rendering/APIAbstractions/Vulkan/CommandPoolVK.hpp>
#include <Engine/Rendering/Window.hpp>

#define NOMINMAX
#define VMA_IMPLEMENTATION
#include <vma/vk_mem_alloc.h>
#include <vulkan/vulkan_win32.h>

DeviceVK::DeviceVK(const DeviceInfoVK& deviceInfo)
    :Device(deviceInfo.QueueFamilyIndices, deviceInfo.pBackBuffer, deviceInfo.pDepthTexture),
    m_Instance(deviceInfo.Instance),
    m_PhysicalDevice(deviceInfo.PhysicalDevice),
    m_Device(deviceInfo.Device),
    m_Allocator(deviceInfo.Allocator),
    m_Surface(deviceInfo.Surface),
    m_SwapchainImages(deviceInfo.SwapchainImages),
    m_SwapchainImageViews(deviceInfo.SwapchainImageViews),
    m_SwapchainFormat(deviceInfo.SwapchainFormat),
    m_DebugMessenger(deviceInfo.DebugMessenger),
    m_QueueHandles(deviceInfo.QueueHandles)
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

    vmaDestroyAllocator(m_Allocator);
    vkDestroySwapchainKHR(m_Device, m_Swapchain, nullptr);
    vkDestroyDevice(m_Device, nullptr);
    vkDestroySurfaceKHR(m_Instance, m_Surface, nullptr);
    vkDestroyInstance(m_Instance, nullptr);
}

bool DeviceVK::graphicsQueueSubmit(ICommandList* pCommandList)
{
    if (!executeCommandBuffer(m_QueueHandles.Graphics, pCommandList)) {
        LOG_WARNING("Failed to submit to graphics queue");
        return false;
    }

    return true;
}

bool DeviceVK::transferQueueSubmit(ICommandList* pCommandList)
{
    if (!executeCommandBuffer(m_QueueHandles.Transfer, pCommandList)) {
        LOG_WARNING("Failed to submit to transfer queue");
        return false;
    }

    return true;
}

bool DeviceVK::computeQueueSubmit(ICommandList* pCommandList)
{
    if (!executeCommandBuffer(m_QueueHandles.Compute, pCommandList)) {
        LOG_WARNING("Failed to submit to compute queue");
        return false;
    }

    return true;
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

ICommandPool* DeviceVK::createCommandPool(COMMAND_POOL_FLAG creationFlags, uint32_t queueFamilyIndex)
{
    return CommandPoolVK::create(creationFlags, queueFamilyIndex, this);
}

bool DeviceVK::executeCommandBuffer(VkQueue queue, ICommandList* pCommandList)
{
    VkCommandBuffer commandBuffer = reinterpret_cast<CommandListVK*>(pCommandList)->getCommandBuffer();

    VkSubmitInfo submitInfo = {};
    submitInfo.sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount   = 1u;
    submitInfo.pCommandBuffers      = &commandBuffer;

    return vkQueueSubmit(VK_NULL_HANDLE, 1u, &submitInfo, VK_NULL_HANDLE) == VK_SUCCESS;
}
