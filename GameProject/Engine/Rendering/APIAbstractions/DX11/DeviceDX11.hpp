#pragma once

#include <Engine/Rendering/APIAbstractions/DX11/BlendStateDX11.hpp>
#include <Engine/Rendering/APIAbstractions/DX11/BufferDX11.hpp>
#include <Engine/Rendering/APIAbstractions/DX11/DepthStencilStateDX11.hpp>
#include <Engine/Rendering/APIAbstractions/DX11/DescriptorPoolDX11.hpp>
#include <Engine/Rendering/APIAbstractions/DX11/FenceDX11.hpp>
#include <Engine/Rendering/APIAbstractions/DX11/PipelineDX11.hpp>
#include <Engine/Rendering/APIAbstractions/DX11/PipelineLayoutDX11.hpp>
#include <Engine/Rendering/APIAbstractions/DX11/RasterizerStateDX11.hpp>
#include <Engine/Rendering/APIAbstractions/DX11/SamplerDX11.hpp>
#include <Engine/Rendering/APIAbstractions/DX11/SemaphoreDX11.hpp>
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

    bool graphicsQueueSubmit(ICommandList* pCommandList, IFence* pFence, SemaphoreSubmitInfo& semaphoreSubmitInfo) override final;
    bool transferQueueSubmit(ICommandList* pCommandList, IFence* pFence, SemaphoreSubmitInfo& semaphoreSubmitInfo) override final;
    bool computeQueueSubmit(ICommandList* pCommandList, IFence* pFence, SemaphoreSubmitInfo& semaphoreSubmitInfo) override final;

    void presentBackBuffer() override final;

    ICommandPool* createCommandPool(COMMAND_POOL_FLAG creationFlags, uint32_t queueFamilyIndex) override final;

    IDescriptorSetLayout* createDescriptorSetLayout() override final;

    IFramebuffer* createFramebuffer(const FramebufferInfo& framebufferInfo) override final;
    IRenderPass* createRenderPass(const RenderPassInfo& renderPassInfo) override final;

    PipelineDX11* createPipeline(const PipelineInfo& pipelineInfo) override final;
    PipelineLayoutDX11* createPipelineLayout(std::vector<IDescriptorSetLayout*> descriptorSetLayout) override final { return DBG_NEW PipelineLayoutDX11(); }

    void map(IBuffer* pBuffer, void** ppMappedMemory) override final;
    void unmap(IBuffer* pBuffer) override final;

    IFence* createFence(bool createSignaled) override final { return DBG_NEW FenceDX11(); }
    ISemaphore* createSemaphore() override final { return DBG_NEW SemaphoreDX11(); }

    // Shader resources
    BufferDX11* createBuffer(const BufferInfo& bufferInfo, StagingResources* pStagingResources = nullptr) override final;

    TextureDX11* createTextureFromFile(const std::string& filePath) override final;
    TextureDX11* createTexture(const TextureInfo& textureInfo) override final;

    SamplerDX11* createSampler(const SamplerInfo& samplerInfo) override final;

    std::string getShaderFileExtension() override final { return ".hlsl"; }

    // Rasterizer
    RasterizerStateDX11* createRasterizerState(const RasterizerStateInfo& rasterizerInfo) override final;

    // Output merger
    BlendStateDX11* createBlendState(const BlendStateInfo& blendStateInfo) override final;
    DepthStencilStateDX11* createDepthStencilState(const DepthStencilInfo& depthStencilInfo) override final;

    bool waitForFences(IFence** ppFences, uint32_t fenceCount, bool waitAll, uint64_t timeout) override final { return true; }

    ID3D11Device* getDevice()           { return m_pDevice; }
    ID3D11DeviceContext* getContext()   { return m_pContext; }

protected:
    DescriptorPoolDX11* createDescriptorPool(const DescriptorCounts& poolSize) override final;

private:
    ShaderDX11* compileShader(SHADER_TYPE shaderType, const std::string& filePath, const InputLayoutInfo* pInputLayoutInfo, InputLayout** ppInputLayout) override final;
    bool executeCommandList(ICommandList* pCommandList);

private:
    ID3D11Device* m_pDevice;
    // Immediate context
    ID3D11DeviceContext* m_pContext;

    IDXGISwapChain* m_pSwapChain;
    ID3D11DepthStencilState* m_pDepthStencilState;

    const FLOAT m_pClearColor[4] = {0.0f, 0.0f, 0.0f, 1.0f};
};
