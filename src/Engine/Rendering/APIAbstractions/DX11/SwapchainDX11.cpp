#include "SwapchainDX11.hpp"

#include <Engine/Utils/DirectXUtils.hpp>

SwapchainDX11::SwapchainDX11(IDXGISwapChain* pSwapchain, Texture* pBackbuffer, const Texture* const * ppDepthTextures)
    :Swapchain(ppDepthTextures),
    m_pSwapchain(pSwapchain),
    m_pBackbuffer(reinterpret_cast<TextureDX11*>(pBackbuffer))
{
    std::memcpy(m_ppDepthTextures, ppDepthTextures, sizeof(Texture*) * MAX_FRAMES_IN_FLIGHT);
}

SwapchainDX11::~SwapchainDX11()
{
    SAFERELEASE(m_pSwapchain)
    delete m_pBackbuffer;
}

void SwapchainDX11::present(ISemaphore** ppWaitSemaphores, uint32_t waitSemaphoreCount)
{
    HRESULT hr = m_pSwapchain->Present(0, 0);
    if (FAILED(hr)) {
        LOG_WARNING("Failed to present swapchain buffer: %s", hresultToString(hr).c_str());
    }
}
