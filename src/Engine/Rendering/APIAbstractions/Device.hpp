#pragma once

#include <Engine/Rendering/APIAbstractions/DescriptorPoolHandler.hpp>
#include <Engine/Rendering/APIAbstractions/GeneralResources.hpp>
#include <Engine/Rendering/APIAbstractions/Swapchain.hpp>
#include <Engine/Rendering/ShaderHandler.hpp>
#include <Engine/Utils/ResourcePool.hpp>

#define NOMINMAX
#include <DirectXMath.h>

#include <array>
#include <string>

class BlendState;
class IBuffer;
class ICommandList;
class IDepthStencilState;
class IFence;
class Framebuffer;
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
    uint32_t WaitSemaphoreCount;
    PIPELINE_STAGE* pWaitStageFlags;
    ISemaphore** ppSignalSemaphores;
    uint32_t SignalSemaphoreCount;
};

class Device
{
public:
    static Device* create(RENDERING_API API, const SwapchainInfo& swapchainInfo, const Window* pWindow);

public:
    Device(QueueFamilyIndices queueFamilyIndices);
    virtual ~Device();

    bool init(const DescriptorCounts& descriptorCounts);

    virtual bool graphicsQueueSubmit(ICommandList* pCommandList, IFence* pFence, const SemaphoreSubmitInfo* pSemaphoreInfo) = 0;
    virtual bool transferQueueSubmit(ICommandList* pCommandList, IFence* pFence, const SemaphoreSubmitInfo* pSemaphoreInfo) = 0;
    virtual bool computeQueueSubmit(ICommandList* pCommandList, IFence* pFence, const SemaphoreSubmitInfo* pSemaphoreInfo) = 0;

    void presentBackbuffer(ISemaphore** ppWaitSemaphores, uint32_t waitSemaphoreCount);

    virtual ICommandPool* createCommandPool(COMMAND_POOL_FLAG creationFlags, uint32_t queueFamilyIndex) = 0;
    PooledResource<ICommandPool> acquireTempCommandPoolGraphics()  { return m_CommandPoolsTempGraphics.acquire(); }
    PooledResource<ICommandPool> acquireTempCommandPoolTransfer()  { return m_CommandPoolsTempTransfer.acquire(); }
    PooledResource<ICommandPool> acquireTempCommandPoolCompute()   { return m_CommandPoolsTempCompute.acquire(); }

    virtual IDescriptorSetLayout* createDescriptorSetLayout() = 0;
    DescriptorSet* allocateDescriptorSet(const IDescriptorSetLayout* pDescriptorSetLayout);

    virtual Framebuffer* createFramebuffer(const FramebufferInfo& framebufferInfo) = 0;
    virtual IRenderPass* createRenderPass(const RenderPassInfo& renderPassInfo) = 0;

    virtual IPipelineLayout* createPipelineLayout(std::vector<IDescriptorSetLayout*> descriptorSetLayouts) = 0;
    virtual IPipeline* createPipeline(const PipelineInfo& pipelineInfo) = 0;

    virtual void map(IBuffer* pBuffer, void** ppMappedMemory) = 0;
    virtual void unmap(IBuffer* pBuffer) = 0;

    virtual IFence* createFence(bool createSignaled) = 0;
    virtual ISemaphore* createSemaphore() = 0;

    // Shader resources
    // pStagingResources can be used if the buffer has initial data and is not CPU-writable.
    // If the staging resources are needed but none are specified, temporary ones are created.
    virtual IBuffer* createBuffer(const BufferInfo& bufferInfo, StagingResources* pStagingResources = nullptr) = 0;
    IBuffer* createVertexBuffer(const void* pVertices, size_t vertexSize, size_t vertexCount);
    IBuffer* createIndexBuffer(const unsigned* pIndices, size_t indexCount);

    virtual Texture* createTextureFromFile(const std::string& filePath) = 0;
    virtual Texture* createTexture(const TextureInfo& textureInfo) = 0;

    virtual ISampler* createSampler(const SamplerInfo& samplerInfo) = 0;

    // Shader's file path excludes the base path to the shaders folder
    Shader* createShader(SHADER_TYPE shaderType, const std::string& filePath, const InputLayoutInfo* pInputLayoutInfo);
    virtual std::string getShaderFileExtension() = 0;

    // waitAll: Wait for every fence or just one. timeout: Nanoseconds
    virtual bool waitForFences(IFence** ppFences, uint32_t fenceCount, bool waitAll, uint64_t timeout) = 0;
    // waitIdle waits for the device to be idle
    virtual void waitIdle() = 0;

    Swapchain* getSwapchain()                               { return m_pSwapchain; }
    Texture* getBackbuffer(uint32_t frameIndex)             { return m_pSwapchain->getBackbuffer(frameIndex); }
    Texture* getDepthStencil(uint32_t frameIndex)           { return m_pSwapchain->getDepthTexture(frameIndex); }
    ShaderHandler* getShaderHandler()                       { return m_pShaderHandler; }
    const QueueFamilyIndices& getQueueFamilyIndices() const { return m_QueueFamilyIndices; }
    inline uint32_t& getFrameIndex()                        { return m_FrameIndex; }

protected:
    friend DescriptorPoolHandler;

    virtual DescriptorPool* createDescriptorPool(const DescriptorPoolInfo& poolInfo) = 0;

    // VkDevice is deleted before the graphics objects in Device are. This is a workaround for that issue.
    void deleteGraphicsObjects();

protected:
    Swapchain* m_pSwapchain;
    uint32_t m_FrameIndex;

    DescriptorPoolHandler m_DescriptorPoolHandler;

    // Each pool contains one command pool for each thread. The generated command lists are temporary, i.e. short lived.
    ResourcePool<ICommandPool> m_CommandPoolsTempGraphics;
    ResourcePool<ICommandPool> m_CommandPoolsTempTransfer;
    ResourcePool<ICommandPool> m_CommandPoolsTempCompute;

private:
    virtual Shader* compileShader(SHADER_TYPE shaderType, const std::string& filePath, const InputLayoutInfo* pInputLayoutInfo) = 0;

    struct TempCommandPoolInfo {
        uint32_t queueFamilyIndex;
        ResourcePool<ICommandPool>* pTargetCommandPool;
    };

    bool initTempCommandPools();
    bool initTempCommandPool(std::vector<ICommandPool*>& commandPools, TempCommandPoolInfo& commandPoolInfo);

    inline void setSwapchain(Swapchain* pSwapchain) { m_pSwapchain = pSwapchain; }

private:
    ShaderHandler* m_pShaderHandler;
    QueueFamilyIndices m_QueueFamilyIndices;
};
