#include "SwapchainVK.hpp"

#include <Engine/Rendering/APIAbstractions/Vulkan/DeviceVK.hpp>
#include <Engine/Rendering/APIAbstractions/Vulkan/SemaphoreVK.hpp>

SwapchainVK::SwapchainVK(VkSwapchainKHR swapchain, const Texture* const * ppBackbuffers, const Texture* const * ppDepthTextures, DeviceVK* pDevice)
    :Swapchain(ppDepthTextures),
    m_Swapchain(swapchain),
    m_pDevice(pDevice)
{
    std::memcpy(m_ppBackbuffers, ppBackbuffers, sizeof(TextureVK*) * MAX_FRAMES_IN_FLIGHT);
}

SwapchainVK::~SwapchainVK()
{
    vkDestroySwapchainKHR(m_pDevice->getDevice(), m_Swapchain, nullptr);
}

void SwapchainVK::present(ISemaphore** ppWaitSemaphores, uint32_t waitSemaphoreCount)
{
    if (waitSemaphoreCount > (uint32_t)m_WaitSemaphores.size()) {
        m_WaitSemaphores.resize((size_t)waitSemaphoreCount);
    }

    for (uint32_t semaphoreIdx = 0u; semaphoreIdx < waitSemaphoreCount; semaphoreIdx += 1u) {
        m_WaitSemaphores[semaphoreIdx] = reinterpret_cast<SemaphoreVK*>(ppWaitSemaphores[semaphoreIdx])->getSemaphore();
    }

    const uint32_t frameIndex = m_pDevice->getFrameIndex();

    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType               = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount  = waitSemaphoreCount;
    presentInfo.pWaitSemaphores     = m_WaitSemaphores.data();
    presentInfo.swapchainCount      = 1u;
    presentInfo.pSwapchains         = &m_Swapchain;
    presentInfo.pImageIndices       = &frameIndex;
    presentInfo.pResults            = nullptr;

    const VkQueue presentQueue = m_pDevice->getQueues().Present;
    if (vkQueuePresentKHR(presentQueue, &presentInfo) != VK_SUCCESS) {
        LOG_WARNING("Failed to present swapchain image");
    }
}
