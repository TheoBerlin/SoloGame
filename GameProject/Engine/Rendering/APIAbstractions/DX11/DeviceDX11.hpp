#pragma once

#define NOMINMAX
#include <Engine/Rendering/APIAbstractions/DX11/BufferDX11.hpp>
#include <Engine/Rendering/APIAbstractions/IDevice.hpp>

#include <d3d11.h>

class DeviceDX11 : public IDevice
{
public:
    DeviceDX11();
    ~DeviceDX11();

    bool init(const SwapChainInfo& swapChainInfo, Window* pWindow) override final;

    void clearBackBuffer() override final;
    void presentBackBuffer() override final;

    BufferDX11* createVertexBuffer(const void* pVertices, size_t vertexSize, size_t vertexCount) override final;
    BufferDX11* createIndexBuffer(const unsigned* pIndices, size_t indexCount) override final;

    ID3D11Device* getDevice()               { return m_pDevice; }
    ID3D11DeviceContext* getContext() { return m_pContext; }

    ID3D11RenderTargetView* getBackBuffer()         { return m_pBackBufferRTV; }
    ID3D11DepthStencilView* getDepthStencilView()   { return m_pDepthStencilView; }

private:
    bool initDeviceAndSwapChain(const SwapChainInfo& swapChainInfo, Window* pWindow);
    bool initBackBuffers(const SwapChainInfo& swapChainInfo, Window* pWindow);

private:
    ID3D11Device* m_pDevice;
    // Immediate context
    ID3D11DeviceContext* m_pContext;

    IDXGISwapChain* m_pSwapChain;

    // Backbuffer textures
    ID3D11RenderTargetView* m_pBackBufferRTV;
    // TODO: Remove this, each renderer should have its own blend state
    ID3D11BlendState* m_pBlendState;

    ID3D11Texture2D* m_pDepthStencilTX;
    ID3D11DepthStencilView* m_pDepthStencilView;
    ID3D11DepthStencilState* m_pDepthStencilState;

    const FLOAT m_pClearColor[4] = {0.0f, 0.0f, 0.0f, 1.0f};
};
