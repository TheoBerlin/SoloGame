#pragma once

#include <Engine/Rendering/APIAbstractions/Device.hpp>
#include <Engine/Rendering/APIAbstractions/Vulkan/BufferVK.hpp>
#include <Engine/Rendering/APIAbstractions/Vulkan/FenceVK.hpp>

#include <vulkan/vulkan.h>

#define NOMINMAX
#include <vma/vk_mem_alloc.h>

class DeviceCreatorVK;
class SwapchainVK;

struct Queues {
    VkQueue Graphics;
    VkQueue Transfer;
    VkQueue Compute;
    VkQueue Present;
};

struct DeviceInfoVK {
    VkInstance Instance;
    VkPhysicalDevice PhysicalDevice;
    VkDevice Device;
    VmaAllocator Allocator;
    VkSurfaceKHR Surface;
    VkDebugUtilsMessengerEXT DebugMessenger;
    QueueFamilyIndices QueueFamilyIndices;
    Queues QueueHandles;
};

class DeviceVK : public Device
{
public:
    DeviceVK(const DeviceInfoVK& deviceInfo);
    ~DeviceVK();

    bool graphicsQueueSubmit(ICommandList* pCommandList, IFence* pFence, SemaphoreSubmitInfo& semaphoreSubmitInfo) override final;
    bool transferQueueSubmit(ICommandList* pCommandList, IFence* pFence, SemaphoreSubmitInfo& semaphoreSubmitInfo) override final;
    bool computeQueueSubmit(ICommandList* pCommandList, IFence* pFence, SemaphoreSubmitInfo& semaphoreSubmitInfo) override final;

    ICommandPool* createCommandPool(COMMAND_POOL_FLAG creationFlags, uint32_t queueFamilyIndex) override final;

    IDescriptorSetLayout* createDescriptorSetLayout() override final;

    Framebuffer* createFramebuffer(const FramebufferInfo& framebufferInfo) override final;
    IRenderPass* createRenderPass(const RenderPassInfo& renderPassInfo) override final;

    IPipelineLayout* createPipelineLayout(std::vector<IDescriptorSetLayout*> descriptorSetLayouts) override final;
    IPipeline* createPipeline(const PipelineInfo& pipelineInfo) override final;

    void map(IBuffer* pBuffer, void** ppMappedMemory) override final;
    void unmap(IBuffer* pBuffer) override final;

    FenceVK* createFence(bool createSignaled) override final;
    ISemaphore* createSemaphore() override final;

    // Shader resources
    BufferVK* createBuffer(const BufferInfo& bufferInfo, StagingResources* pStagingResources = nullptr) override final;
    BufferVK* createStagingBuffer(const void* pData, VkDeviceSize size);

    Texture* createTextureFromFile(const std::string& filePath) override final;
    Texture* createTexture(const TextureInfo& textureInfo) override final;

    ISampler* createSampler(const SamplerInfo& samplerInfo) override final;

    std::string getShaderFileExtension() override final { return ".spv"; }

    bool waitForFences(IFence** ppFences, uint32_t fenceCount, bool waitAll, uint64_t timeout) override final;

    VmaAllocator getVulkanAllocator()   { return m_Allocator; }
    VkDevice getDevice()                { return m_Device; }
    const Queues& getQueues() const     { return m_QueueHandles; }

protected:
    DescriptorPool* createDescriptorPool(const DescriptorPoolInfo& poolInfo) override final;

private:
    friend DeviceCreatorVK;
    static VKAPI_ATTR VkBool32 VKAPI_CALL vulkanCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);

private:
    Shader* compileShader(SHADER_TYPE shaderType, const std::string& filePath, const InputLayoutInfo* pInputLayoutInfo) override final;
    bool executeCommandBuffer(VkQueue queue, ICommandList* pCommandList, IFence* pFence, SemaphoreSubmitInfo& semaphoreSubmitInfo);

private:
    VkInstance m_Instance;
    VkPhysicalDevice m_PhysicalDevice;
    VkDevice m_Device;
    VkSurfaceKHR m_Surface;
    VmaAllocator m_Allocator;

    VkDebugUtilsMessengerEXT m_DebugMessenger;

    Queues m_QueueHandles;
};
