#pragma once

#include <Engine/Rendering/APIAbstractions/DeviceCreator.hpp>

#include <d3d11.h>

class TextureDX11;

class DeviceCreatorDX11 : public IDeviceCreator
{
public:
    DeviceCreatorDX11() = default;
    ~DeviceCreatorDX11() = default;

    Device* createDevice(const SwapchainInfo& swapChainInfo, const Window* pWindow) override final;
    Swapchain* createSwapchain(Device* pDevice) override final;

private:
    bool initDeviceAndSwapChain(const SwapchainInfo& swapChainInfo, const Window* pWindow);
    bool initBackBuffers(const SwapchainInfo& swapChainInfo, const Window* pWindow);

private:
    ID3D11Device* m_pDevice;
    // Immediate context
    ID3D11DeviceContext* m_pContext;

    IDXGISwapChain* m_pSwapChain;
    ID3D11DepthStencilState* m_pDepthStencilState;

    Texture* m_pBackbuffer;
    Texture* m_ppDepthTextures[MAX_FRAMES_IN_FLIGHT];
};
