#include "SemaphoreVK.hpp"

#include <Engine/Rendering/APIAbstractions/Vulkan/DeviceVK.hpp>

SemaphoreVK* SemaphoreVK::create(DeviceVK* pDevice)
{
    VkSemaphoreCreateInfo semaphoreInfo = {};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkSemaphore semaphore = VK_NULL_HANDLE;
    if (vkCreateSemaphore(pDevice->getDevice(), &semaphoreInfo, nullptr, &semaphore) != VK_SUCCESS) {
        LOG_ERROR("Failed to create semaphore");
        return nullptr;
    }

    return DBG_NEW SemaphoreVK(semaphore, pDevice);
}

SemaphoreVK::SemaphoreVK(VkSemaphore semaphore, DeviceVK* pDevice)
    :m_Semaphore(semaphore),
    m_pDevice(pDevice)
{}

SemaphoreVK::~SemaphoreVK()
{
    vkDestroySemaphore(m_pDevice->getDevice(), m_Semaphore, nullptr);
}
