#include "CommandListVK.hpp"

#include <Engine/Rendering/APIAbstractions/Vulkan/DeviceVK.hpp>

CommandListVK::CommandListVK(VkCommandBuffer commandBuffer, VkCommandPool commandPool, DeviceVK* pDevice)
    :m_CommandBuffer(commandBuffer),
    m_CommandPool(commandPool),
    m_pDevice(pDevice)
{}

CommandListVK::~CommandListVK()
{
    vkFreeCommandBuffers(m_pDevice->getDevice(), m_CommandPool, 1u, &m_CommandBuffer);
}
