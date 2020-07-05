#pragma once

#include <Engine/Rendering/APIAbstractions/Device.hpp>
#include <Engine/Rendering/APIAbstractions/DX11/TextureDX11.hpp>
#include <Engine/Rendering/APIAbstractions/Swapchain.hpp>

#define NOMINMAX
#include <d3d11.h>

class TextureDX11;

class SwapchainDX11 : public Swapchain
{
public:
    SwapchainDX11(IDXGISwapChain* pSwapchain, Texture* pBackbuffer, const Texture* const * ppDepthTextures);
    ~SwapchainDX11();

    void present(ISemaphore** ppWaitSemaphores, uint32_t waitSemaphoreCount) override final;

    Texture* getBackbuffer(uint32_t frameIndex) override final { return m_pBackbuffer; }

private:
    IDXGISwapChain* m_pSwapchain;

    // DirectX 11 only exposes one texture pointer for all swapchain buffers
    TextureDX11* m_pBackbuffer;
};
