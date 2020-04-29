#pragma once

#define NOMINMAX
#include <Engine/Rendering/APIAbstractions/Device.hpp>

#include <d3d11.h>

class DeviceDX11 : public Device
{
public:
    DeviceDX11();
    ~DeviceDX11();

    bool init(const SwapChainInfo& swapChainInfo, Window* pWindow) override final;

private:
    bool initDeviceAndSwapChain(const SwapChainInfo& swapChainInfo, Window* pWindow);
    bool initBackBuffers(const SwapChainInfo& swapChainInfo, Window* pWindow);

private:
    ID3D11Device* m_pDevice;
    IDXGISwapChain* m_pSwapChain;

    // Immediate context
    ID3D11DeviceContext* m_pDeviceContext;

    // Backbuffer textures
    ID3D11RenderTargetView* m_pBackBufferRTV;
    // TODO: Remove this, each renderer should have its own blend state
    ID3D11BlendState* m_pBlendState;

    ID3D11Texture2D* m_pDepthStencilTX;
    ID3D11DepthStencilView* m_pDepthStencilView;
    ID3D11DepthStencilState* m_pDepthStencilState;
};
