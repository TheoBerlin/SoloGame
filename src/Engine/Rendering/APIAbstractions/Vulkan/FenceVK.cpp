#include "FenceVK.hpp"

#include <Engine/Rendering/APIAbstractions/Vulkan/DeviceVK.hpp>

FenceVK* FenceVK::create(bool createSignaled, DeviceVK* pDevice)
{
    VkFenceCreateInfo fenceInfo = {};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = createSignaled ? VK_FENCE_CREATE_FLAG_BITS_MAX_ENUM : 0u;

    VkFence fence = VK_NULL_HANDLE;
    if (vkCreateFence(pDevice->getDevice(), &fenceInfo, nullptr, &fence) != VK_SUCCESS) {
        LOG_ERROR("Failed to create fence");
        return nullptr;
    }

    return DBG_NEW FenceVK(fence, pDevice);
}

FenceVK::FenceVK(VkFence fence, DeviceVK* pDevice)
    :m_Fence(fence),
    m_pDevice(pDevice)
{}

FenceVK::~FenceVK()
{
    vkDestroyFence(m_pDevice->getDevice(), m_Fence, nullptr);
}

bool FenceVK::isSignaled()
{
    return vkGetFenceStatus(m_pDevice->getDevice(), m_Fence) == VK_SUCCESS;
};
