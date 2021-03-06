#include "Swapchain.hpp"

Swapchain::Swapchain(const Texture* const * ppDepthTextures, const ISemaphore* const * ppSemaphores, IFence* pFence)
    :m_pFence(pFence),
    m_PreviousFrameIndex(0u)
{
    std::memcpy(m_ppDepthTextures, ppDepthTextures, sizeof(Texture*) * MAX_FRAMES_IN_FLIGHT);
    std::memcpy(m_ppSemaphores, ppSemaphores, sizeof(ISemaphore*) * MAX_FRAMES_IN_FLIGHT);
}

Swapchain::~Swapchain()
{
    for (uint32_t frameIndex = 0u; frameIndex < MAX_FRAMES_IN_FLIGHT; frameIndex += 1u) {
        delete m_ppDepthTextures[frameIndex];
        delete m_ppSemaphores[frameIndex];
    }

    delete m_pFence;
}

bool Swapchain::acquireNextBackbuffer(uint32_t& frameIndex, SYNC_OPTION syncOptions)
{
    m_PreviousFrameIndex = frameIndex;
    return dAcquireNextBackbuffer(frameIndex, syncOptions);
}
