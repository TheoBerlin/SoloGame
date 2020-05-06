#pragma once

#define NOMINMAX
#include <Engine/Rendering/APIAbstractions/DX11/BufferDX11.hpp>
#include <Engine/Rendering/APIAbstractions/DX11/SamplerDX11.hpp>
#include <Engine/Rendering/APIAbstractions/DX11/TextureDX11.hpp>
#include <Engine/Rendering/APIAbstractions/Device.hpp>

#include <d3d11.h>

class DeviceDX11 : public Device
{
public:
    DeviceDX11();
    ~DeviceDX11();

    bool init(const SwapChainInfo& swapChainInfo, Window* pWindow) override final;

    void clearBackBuffer() override final;
    void presentBackBuffer() override final;

    ICommandList* createCommandList() override final;

    BufferDX11* createBuffer(const BufferInfo& bufferInfo) override final;
    BufferDX11* createVertexBuffer(const void* pVertices, size_t vertexSize, size_t vertexCount) override final;
    BufferDX11* createIndexBuffer(const unsigned* pIndices, size_t indexCount) override final;

    TextureDX11* createTextureFromFile(const std::string& filePath) override final;
    TextureDX11* createTexture(const TextureInfo& textureInfo) override final;

    IRasterizerState* createRasterizerState(const RasterizerStateInfo& rasterizerInfo) override final;

    SamplerDX11* createSampler(const SamplerInfo& samplerInfo) override final;

    ID3D11Device* getDevice()           { return m_pDevice; }
    ID3D11DeviceContext* getContext()   { return m_pContext; }

private:
    bool initDeviceAndSwapChain(const SwapChainInfo& swapChainInfo, Window* pWindow);
    bool initBackBuffers(const SwapChainInfo& swapChainInfo, Window* pWindow);

private:
    ID3D11Device* m_pDevice;
    // Immediate context
    ID3D11DeviceContext* m_pContext;

    IDXGISwapChain* m_pSwapChain;

    // TODO: Remove this, each renderer should have its own blend state
    ID3D11BlendState* m_pBlendState;

    ID3D11DepthStencilState* m_pDepthStencilState;

    const FLOAT m_pClearColor[4] = {0.0f, 0.0f, 0.0f, 1.0f};
};
