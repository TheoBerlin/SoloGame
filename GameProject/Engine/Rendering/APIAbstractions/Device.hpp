#pragma once

#include <Engine/Rendering/APIAbstractions/DescriptorPoolHandler.hpp>
#include <Engine/Rendering/APIAbstractions/GeneralResources.hpp>
#include <Engine/Rendering/ShaderHandler.hpp>
#include <Engine/Utils/ResourcePool.hpp>

#define NOMINMAX
#include <DirectXMath.h>

#include <string>

class BlendState;
class IBuffer;
class ICommandList;
class IDepthStencilState;
class IFence;
class IFramebuffer;
class InputLayout;
class IPipeline;
class IPipelineLayout;
class IRasterizerState;
class IRenderPass;
class ISampler;
class ISemaphore;
class Texture;
class Window;
struct BlendStateInfo;
struct BufferInfo;
struct DepthStencilInfo;
struct InputLayoutInfo;
struct FramebufferInfo;
struct PipelineInfo;
struct RasterizerStateInfo;
struct RenderPassInfo;
struct SamplerInfo;
struct StagingResources;
struct TextureInfo;

struct SwapchainInfo {
    uint32_t FrameRateLimit;
    uint32_t Multisamples;
    bool Windowed;
};

enum class RENDERING_API {
    DIRECTX11,
    VULKAN
};

struct QueueFamilyIndices {
    uint32_t Graphics;
    uint32_t Transfer;
    uint32_t Compute;
    uint32_t Present;
};

struct SemaphoreSubmitInfo {
    ISemaphore** ppWaitSemaphores;
    uint32_t waitSemaphoreCount;
    PIPELINE_STAGE* pWaitStageFlags;
    ISemaphore** ppSignalSemaphores;
    uint32_t signalSemaphoreCount;
};

class Device
{
public:
    static Device* create(RENDERING_API API, const SwapchainInfo& swapchainInfo, const Window* pWindow);

public:
    Device(QueueFamilyIndices queueFamilyIndices, Texture* pBackBuffer, Texture* pDepthTexture);
    virtual ~Device();

    bool init(const DescriptorCounts& descriptorCounts);

    virtual bool graphicsQueueSubmit(ICommandList* pCommandList, IFence* pFence, SemaphoreSubmitInfo& semaphoreSubmitInfo) = 0;
    virtual bool transferQueueSubmit(ICommandList* pCommandList, IFence* pFence, SemaphoreSubmitInfo& semaphoreSubmitInfo) = 0;
    virtual bool computeQueueSubmit(ICommandList* pCommandList, IFence* pFence, SemaphoreSubmitInfo& semaphoreSubmitInfo) = 0;

    virtual void presentBackBuffer() = 0;

    virtual ICommandPool* createCommandPool(COMMAND_POOL_FLAG creationFlags, uint32_t queueFamilyIndex) = 0;
    PooledResource<ICommandPool> acquireTempCommandPoolGraphics()  {return m_CommandPoolsTempGraphics.acquire(); }
    PooledResource<ICommandPool> acquireTempCommandPoolTransfer()  {return m_CommandPoolsTempTransfer.acquire(); }
    PooledResource<ICommandPool> acquireTempCommandPoolCompute()   {return m_CommandPoolsTempCompute.acquire(); }

    virtual IDescriptorSetLayout* createDescriptorSetLayout() = 0;
    DescriptorSet* allocateDescriptorSet(const IDescriptorSetLayout* pDescriptorSetLayout);

    virtual IFramebuffer* createFramebuffer(const FramebufferInfo& framebufferInfo) = 0;
    virtual IRenderPass* createRenderPass(const RenderPassInfo& renderPassInfo) = 0;

    virtual IPipelineLayout* createPipelineLayout(std::vector<IDescriptorSetLayout*> descriptorSetLayout) = 0;
    virtual IPipeline* createPipeline(const PipelineInfo& pipelineInfo) = 0;

    virtual void map(IBuffer* pBuffer, void** ppMappedMemory) = 0;
    virtual void unmap(IBuffer* pBuffer) = 0;

    virtual IFence* createFence(bool createSignaled) = 0;
    virtual ISemaphore* createSemaphore() = 0;

    // Shader resources
    // pStagingResources is only needed if the buffer has initial data and is not CPU-writable
    virtual IBuffer* createBuffer(const BufferInfo& bufferInfo, StagingResources* pStagingResources = nullptr) = 0;
    IBuffer* createVertexBuffer(const void* pVertices, size_t vertexSize, size_t vertexCount);
    IBuffer* createIndexBuffer(const unsigned* pIndices, size_t indexCount);

    virtual Texture* createTextureFromFile(const std::string& filePath) = 0;
    virtual Texture* createTexture(const TextureInfo& textureInfo) = 0;

    virtual ISampler* createSampler(const SamplerInfo& samplerInfo) = 0;

    // Shader's file path excludes the base path to the shaders folder
    Shader* createShader(SHADER_TYPE shaderType, const std::string& filePath, const InputLayoutInfo* pInputLayoutInfo = nullptr, InputLayout** ppInputLayout = nullptr);
    virtual std::string getShaderPostfixAndExtension(SHADER_TYPE shaderType) = 0;

    // Rasterizer
    virtual IRasterizerState* createRasterizerState(const RasterizerStateInfo& rasterizerInfo) = 0;

    // Output merger
    virtual BlendState* createBlendState(const BlendStateInfo& blendStateInfo) = 0;
    virtual IDepthStencilState* createDepthStencilState(const DepthStencilInfo& depthStencilInfo) = 0;

    // waitAll: Wait for every fence or just one. timeout: Nanoseconds
    virtual bool waitForFences(IFence** ppFences, uint32_t fenceCount, bool waitAll, uint64_t timeout) = 0;

    Texture* getBackBuffer()            { return m_pBackBuffer; }
    Texture* getDepthStencil()          { return m_pDepthTexture; }
    ShaderHandler* getShaderHandler()   { return m_pShaderHandler; }
    const QueueFamilyIndices& getQueueFamilyIndices() const { return m_QueueFamilyIndices; }

protected:
    friend DescriptorPoolHandler;

    virtual DescriptorPool* createDescriptorPool(const DescriptorCounts& poolSize) = 0;

protected:
    DescriptorPoolHandler m_DescriptorPoolHandler;

    Texture* m_pBackBuffer;
    Texture* m_pDepthTexture;

private:
    virtual Shader* compileShader(SHADER_TYPE shaderType, const std::string& filePath, const InputLayoutInfo* pInputLayoutInfo, InputLayout** ppInputLayout) = 0;

    struct TempCommandPoolInfo {
        uint32_t queueFamilyIndex;
        ResourcePool<ICommandPool>* pTargetCommandPool;
    };

    bool initTempCommandPools();
    bool initTempCommandPool(std::vector<ICommandPool*>& commandPools, TempCommandPoolInfo& commandPoolInfo);

private:
    ShaderHandler* m_pShaderHandler;
    QueueFamilyIndices m_QueueFamilyIndices;

    // Each pool contains one command pool for each thread. The generated command lists are temporary, i.e. short lived.
    ResourcePool<ICommandPool> m_CommandPoolsTempGraphics;
    ResourcePool<ICommandPool> m_CommandPoolsTempTransfer;
    ResourcePool<ICommandPool> m_CommandPoolsTempCompute;
};
