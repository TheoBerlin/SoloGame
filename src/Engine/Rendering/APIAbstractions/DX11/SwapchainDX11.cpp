#include "SwapchainDX11.hpp"

#include <Engine/Utils/DirectXUtils.hpp>

SwapchainDX11* SwapchainDX11::create(const SwapchainInfoDX11& swapchainInfo, DeviceDX11* pDevice)
{
    ISemaphore* ppSemaphores[MAX_FRAMES_IN_FLIGHT];
    for (uint32_t frameIndex = 0u; frameIndex < MAX_FRAMES_IN_FLIGHT; frameIndex += 1u) {
        ppSemaphores[frameIndex] = pDevice->createSemaphore();
        if (!ppSemaphores[frameIndex]) {
            return nullptr;
        }
    }

    IFence* pFence = pDevice->createFence(true);
    if (!pFence) {
        return nullptr;
    }

    return DBG_NEW SwapchainDX11(swapchainInfo, ppSemaphores, pFence);
}

SwapchainDX11::SwapchainDX11(const SwapchainInfoDX11& swapchainInfo, const ISemaphore* const * ppSemaphores, IFence* pFence)
    :Swapchain(swapchainInfo.ppDepthTextures, ppSemaphores, pFence),
    m_pSwapchain(swapchainInfo.pSwapchain),
    m_pBackbuffer(reinterpret_cast<TextureDX11*>(swapchainInfo.pBackbuffer))
{}

SwapchainDX11::~SwapchainDX11()
{
    SAFERELEASE(m_pSwapchain)
    delete m_pBackbuffer;
}

bool SwapchainDX11::acquireNextBackbuffer(uint32_t& frameIndex, SYNC_OPTION syncOptions)
{
    frameIndex = (frameIndex + 1u) % MAX_FRAMES_IN_FLIGHT;
    return true;
}

void SwapchainDX11::present(ISemaphore** ppWaitSemaphores, uint32_t waitSemaphoreCount)
{
    HRESULT hr = m_pSwapchain->Present(0, 0);
    if (FAILED(hr)) {
        LOG_WARNING("Failed to present swapchain buffer: %s", hresultToString(hr).c_str());
    }
}
