#include "CommandPoolVK.hpp"

#include <Engine/Rendering/APIAbstractions/Vulkan/CommandListVK.hpp>
#include <Engine/Rendering/APIAbstractions/Vulkan/DeviceVK.hpp>

CommandPoolVK* CommandPoolVK::create(COMMAND_POOL_FLAG creationFlags, uint32_t queueFamilyIndex, DeviceVK* pDevice)
{
    VkCommandPoolCreateInfo poolInfo = {};
    poolInfo.sType  = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags  =   HAS_FLAG(creationFlags, COMMAND_POOL_FLAG::TEMPORARY_COMMAND_LISTS) * VK_COMMAND_POOL_CREATE_TRANSIENT_BIT |
                        HAS_FLAG(creationFlags, COMMAND_POOL_FLAG::RESETTABLE_COMMAND_LISTS) * VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = queueFamilyIndex;

    VkCommandPool commandPool = VK_NULL_HANDLE;
    if (vkCreateCommandPool(pDevice->getDevice(), &poolInfo, nullptr, &commandPool) != VK_SUCCESS) {
        LOG_ERROR("Failed to create command pool");
        return nullptr;
    }

    return DBG_NEW CommandPoolVK(commandPool, pDevice);
}

CommandPoolVK::CommandPoolVK(VkCommandPool commandPool, DeviceVK* pDevice)
    :m_CommandPool(commandPool),
    m_pDevice(pDevice)
{}

CommandPoolVK::~CommandPoolVK()
{
    vkDestroyCommandPool(m_pDevice->getDevice(), m_CommandPool, nullptr);
}

bool CommandPoolVK::allocateCommandLists(ICommandList** ppCommandLists, uint32_t commandListCount, COMMAND_LIST_LEVEL level)
{
    std::vector<VkCommandBuffer> commandBuffers((size_t)commandListCount);

    VkCommandBufferAllocateInfo allocationInfo = {};
    allocationInfo.sType                = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocationInfo.commandPool          = m_CommandPool;
    allocationInfo.level                = level == COMMAND_LIST_LEVEL::PRIMARY ? VK_COMMAND_BUFFER_LEVEL_PRIMARY : VK_COMMAND_BUFFER_LEVEL_SECONDARY;
    allocationInfo.commandBufferCount   = commandListCount;
    if (vkAllocateCommandBuffers(m_pDevice->getDevice(), nullptr, commandBuffers.data()) != VK_SUCCESS) {
        LOG_ERROR("Failed to allocate [%d] command list(s)", commandListCount);
        return false;
    }

    for (uint32_t commandListIdx = 0u; commandListIdx < commandListCount; commandListIdx++) {
        ppCommandLists[commandListIdx] = DBG_NEW CommandListVK(commandBuffers[commandListIdx], m_CommandPool, m_pDevice);
    }

    return true;
}
