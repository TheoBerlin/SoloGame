#pragma once

#include <vulkan/vulkan.h>

class DeviceCreatorVK;
struct VmaAllocator_T;

#define NOMINMAX
#include <vma/vk_mem_alloc.h>

struct QueueFamilyIndices {
    uint32_t GraphicsFamily;
    uint32_t TransferFamily;
    uint32_t ComputeFamily;
    uint32_t PresentFamily;
};

struct SwapchainSupportDetails {
    VkSurfaceCapabilitiesKHR SurfaceCapabilities;
    std::vector<VkSurfaceFormatKHR> Formats;
    std::vector<VkPresentModeKHR> PresentModes;
};


struct DeviceInfoVK {
    Texture* pBackBuffer;
    Texture* pDepthTexture;
    VkInstance Instance;
    VkPhysicalDevice PhysicalDevice;
    VkDevice Device;
    VmaAllocator Allocator;
    VkSurfaceKHR Surface;
    VkSwapchainKHR Swapchain;
    std::vector<VkImage> SwapchainImages;
    std::vector<VkImageView> SwapchainImageViews;
    VkSurfaceFormatKHR SwapchainFormat;
    VkDebugUtilsMessengerEXT DebugMessenger;
    QueueFamilyIndices QueueFamilyIndices;
};

class DeviceVK : public Device
{
public:
    DeviceVK(const DeviceInfoVK& deviceInfo);
    ~DeviceVK();

    void presentBackBuffer() override final {};

    ICommandList* createCommandList() override final { return nullptr; }

    IDescriptorSetLayout* createDescriptorSetLayout() override final { return nullptr; }

    IFramebuffer* createFramebuffer(const FramebufferInfo& framebufferInfo) override final  { return nullptr; }
    IRenderPass* createRenderPass(const RenderPassInfo& renderPassInfo) override final      { return nullptr; }

    IPipelineLayout* createPipelineLayout(std::vector<IDescriptorSetLayout*> descriptorSetLayout) override final { return nullptr; }
    IPipeline* createPipeline(const PipelineInfo& pipelineInfo) override final { return nullptr; }

    IBuffer* createBuffer(const BufferInfo& bufferInfo) override final                                          { return nullptr; }
    IBuffer* createVertexBuffer(const void* pVertices, size_t vertexSize, size_t vertexCount) override final    { return nullptr; }
    IBuffer* createIndexBuffer(const unsigned* pIndices, size_t indexCount) override final                      { return nullptr; }

    Texture* createTextureFromFile(const std::string& filePath) override final  { return nullptr; }
    Texture* createTexture(const TextureInfo& textureInfo) override final       { return nullptr; }

    ISampler* createSampler(const SamplerInfo& samplerInfo) override final      { return nullptr; }

    std::string getShaderPostfixAndExtension(SHADER_TYPE shaderType)            { return ""; }

    IRasterizerState* createRasterizerState(const RasterizerStateInfo& rasterizerInfo) override final       { return nullptr; }

    BlendState* createBlendState(const BlendStateInfo& blendStateInfo) override final                       { return nullptr; }
    IDepthStencilState* createDepthStencilState(const DepthStencilInfo& depthStencilInfo) override final    { return nullptr; }

protected:
    DescriptorPool* createDescriptorPool(const DescriptorCounts& poolSize) override final { return nullptr; }

private:
    friend DeviceCreatorVK;
    static VKAPI_ATTR VkBool32 VKAPI_CALL vulkanCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);

private:
    Shader* compileShader(SHADER_TYPE shaderType, const std::string& filePath, const InputLayoutInfo* pInputLayoutInfo, InputLayout** ppInputLayout) override final { return nullptr; }

    bool allocateBuffer(VkBuffer buffer, VkDevice device, const BufferInfo& bufferInfo);

private:
    VkInstance m_Instance;
    VkPhysicalDevice m_PhysicalDevice;
    VkDevice m_Device;
    VmaAllocator m_Allocator;

    VkSurfaceKHR m_Surface;
    VkSwapchainKHR m_Swapchain;
    std::vector<VkImage> m_SwapchainImages;
    std::vector<VkImageView> m_SwapchainImageViews;

    VkSurfaceFormatKHR m_SwapchainFormat;

    VkDebugUtilsMessengerEXT m_DebugMessenger;

    QueueFamilyIndices m_QueueFamilyIndices;
};
