#pragma once

#include <Engine/Rendering/APIAbstractions/DX11/BlendStateDX11.hpp>
#include <Engine/Rendering/APIAbstractions/DX11/BufferDX11.hpp>
#include <Engine/Rendering/APIAbstractions/DX11/DepthStencilStateDX11.hpp>
#include <Engine/Rendering/APIAbstractions/DX11/DescriptorPoolDX11.hpp>
#include <Engine/Rendering/APIAbstractions/DX11/PipelineDX11.hpp>
#include <Engine/Rendering/APIAbstractions/DX11/PipelineLayoutDX11.hpp>
#include <Engine/Rendering/APIAbstractions/DX11/RasterizerStateDX11.hpp>
#include <Engine/Rendering/APIAbstractions/DX11/SamplerDX11.hpp>
#include <Engine/Rendering/APIAbstractions/DX11/ShaderDX11.hpp>
#include <Engine/Rendering/APIAbstractions/DX11/TextureDX11.hpp>
#include <Engine/Rendering/APIAbstractions/Device.hpp>
#include <Engine/Utils/Debug.hpp>

#define NOMINMAX
#include <d3d11.h>

struct DeviceInfoDX11 {
    Texture* pBackBuffer;
    Texture* pDepthTexture;
    ID3D11Device* pDevice;
    ID3D11DeviceContext* pImmediateContext;
    IDXGISwapChain* pSwapChain;
    ID3D11DepthStencilState* pDepthStencilState;
};

class DeviceDX11 : public Device
{
public:
    DeviceDX11(const DeviceInfoDX11& deviceInfo);
    ~DeviceDX11();

    void presentBackBuffer() override final;

    ICommandList* createCommandList() override final;

    IDescriptorSetLayout* createDescriptorSetLayout() override final;

    IFramebuffer* createFramebuffer(const FramebufferInfo& framebufferInfo) override final;
    IRenderPass* createRenderPass(const RenderPassInfo& renderPassInfo) override final;

    PipelineDX11* createPipeline(const PipelineInfo& pipelineInfo) override final;
    PipelineLayoutDX11* createPipelineLayout(std::vector<IDescriptorSetLayout*> descriptorSetLayout) override final { return DBG_NEW PipelineLayoutDX11(); }

    // Shader resources
    BufferDX11* createBuffer(const BufferInfo& bufferInfo) override final;
    BufferDX11* createVertexBuffer(const void* pVertices, size_t vertexSize, size_t vertexCount) override final;
    BufferDX11* createIndexBuffer(const unsigned* pIndices, size_t indexCount) override final;

    TextureDX11* createTextureFromFile(const std::string& filePath) override final;
    TextureDX11* createTexture(const TextureInfo& textureInfo) override final;

    SamplerDX11* createSampler(const SamplerInfo& samplerInfo) override final;

    std::string getShaderPostfixAndExtension(SHADER_TYPE shaderType) override final;

    // Rasterizer
    RasterizerStateDX11* createRasterizerState(const RasterizerStateInfo& rasterizerInfo) override final;

    // Output merger
    BlendStateDX11* createBlendState(const BlendStateInfo& blendStateInfo) override final;
    DepthStencilStateDX11* createDepthStencilState(const DepthStencilInfo& depthStencilInfo) override final;

    ID3D11Device* getDevice()           { return m_pDevice; }
    ID3D11DeviceContext* getContext()   { return m_pContext; }

protected:
    DescriptorPoolDX11* createDescriptorPool(const DescriptorCounts& poolSize) override final;

private:
    ShaderDX11* compileShader(SHADER_TYPE shaderType, const std::string& filePath, const InputLayoutInfo* pInputLayoutInfo, InputLayout** ppInputLayout) override final;

private:
    ID3D11Device* m_pDevice;
    // Immediate context
    ID3D11DeviceContext* m_pContext;

    IDXGISwapChain* m_pSwapChain;
    ID3D11DepthStencilState* m_pDepthStencilState;

    const FLOAT m_pClearColor[4] = {0.0f, 0.0f, 0.0f, 1.0f};
};
