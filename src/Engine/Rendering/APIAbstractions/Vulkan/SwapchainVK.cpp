#include "SwapchainVK.hpp"

#include <Engine/Rendering/APIAbstractions/Vulkan/DeviceVK.hpp>
#include <Engine/Rendering/APIAbstractions/Vulkan/SemaphoreVK.hpp>

SwapchainVK* SwapchainVK::create(const SwapchainInfoVK& swapchainInfo)
{
    ISemaphore* ppSemaphores[MAX_FRAMES_IN_FLIGHT];
    for (uint32_t frameIndex = 0u; frameIndex < MAX_FRAMES_IN_FLIGHT; frameIndex += 1u) {
        ppSemaphores[frameIndex] = swapchainInfo.pDevice->createSemaphore();
        if (!ppSemaphores[frameIndex]) {
            return nullptr;
        }
    }

    FenceVK* pFence = swapchainInfo.pDevice->createFence(true);
    if (!pFence) {
        return nullptr;
    }

    return DBG_NEW SwapchainVK(swapchainInfo, ppSemaphores, pFence);
}

SwapchainVK::SwapchainVK(const SwapchainInfoVK& swapchainInfo, const ISemaphore* const * ppSemaphores, IFence* pFence)
    :Swapchain(swapchainInfo.ppDepthTextures, ppSemaphores, pFence),
    m_Swapchain(swapchainInfo.Swapchain),
    m_pDevice(swapchainInfo.pDevice)
{
    std::memcpy(m_ppBackbuffers, swapchainInfo.ppBackbuffers, sizeof(TextureVK*) * MAX_FRAMES_IN_FLIGHT);
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

bool SwapchainVK::dAcquireNextBackbuffer(uint32_t& frameIndex, SYNC_OPTION syncOptions)
{
    VkSemaphore semaphore = VK_NULL_HANDLE;
    VkFence fence = VK_NULL_HANDLE;

    if (HAS_FLAG(syncOptions, SYNC_OPTION::FENCE)) {
        FenceVK* pFence = reinterpret_cast<FenceVK*>(m_pFence);
        if (!pFence->reset()) {
            return false;
        }

        fence = pFence->getFence();
    }

    if (HAS_FLAG(syncOptions, SYNC_OPTION::SEMAPHORE)) {
        semaphore = reinterpret_cast<SemaphoreVK*>(m_ppSemaphores[frameIndex])->getSemaphore();
    }

    if (vkAcquireNextImageKHR(m_pDevice->getDevice(), m_Swapchain, UINT64_MAX, semaphore, fence, &frameIndex) != VK_SUCCESS) {
        LOG_WARNING("Failed to acquire next swapchain image");
        return false;
    }

    return true;
}
