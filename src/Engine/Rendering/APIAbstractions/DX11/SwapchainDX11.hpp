#pragma once

#include <Engine/Rendering/APIAbstractions/DX11/DeviceDX11.hpp>
#include <Engine/Rendering/APIAbstractions/DX11/TextureDX11.hpp>
#include <Engine/Rendering/APIAbstractions/Swapchain.hpp>

#define NOMINMAX
#include <d3d11.h>

class TextureDX11;

struct SwapchainInfoDX11 {
    IDXGISwapChain* pSwapchain;
    Texture* pBackbuffer;
    const Texture* const * ppDepthTextures;
};

class SwapchainDX11 : public Swapchain
{
public:
    static SwapchainDX11* create(const SwapchainInfoDX11& swapchainInfo, DeviceDX11* pDevice);

public:
    SwapchainDX11(const SwapchainInfoDX11& swapchainInfo, const ISemaphore* const * ppSemaphores, IFence* pFence);
    ~SwapchainDX11();

    bool acquireNextBackbuffer(uint32_t& frameIndex, SYNC_OPTION syncOptions) override final;
    void present(ISemaphore** ppWaitSemaphores, uint32_t waitSemaphoreCount) override final;

    Texture* getBackbuffer(uint32_t frameIndex) override final { return m_pBackbuffer; }

private:
    IDXGISwapChain* m_pSwapchain;

    // DirectX 11 only exposes one texture pointer for all swapchain buffers
    TextureDX11* m_pBackbuffer;
};
