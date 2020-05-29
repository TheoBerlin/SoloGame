#pragma once

#include <vulkan/vulkan.h>

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

class DeviceVK : public Device
{
public:
    DeviceVK();
    ~DeviceVK();

    bool init(const SwapchainInfo& swapchainInfo, Window* pWindow) override final;

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

    std::string getShaderPostfixAndExtension(SHADER_TYPE shaderType) { return ""; }

    IRasterizerState* createRasterizerState(const RasterizerStateInfo& rasterizerInfo) override final       { return nullptr; }

    BlendState* createBlendState(const BlendStateInfo& blendStateInfo) override final                       { return nullptr; }
    IDepthStencilState* createDepthStencilState(const DepthStencilInfo& depthStencilInfo) override final    { return nullptr; }

protected:
    DescriptorPool* createDescriptorPool(const DescriptorCounts& poolSize) override final { return nullptr; }

private:
    static VKAPI_ATTR VkBool32 VKAPI_CALL vulkanCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);

private:
    Shader* compileShader(SHADER_TYPE shaderType, const std::string& filePath, const InputLayoutInfo* pInputLayoutInfo, InputLayout** ppInputLayout) override final { return nullptr; }

private:
    bool initInstance(Window* pWindow, bool debugMode);
    bool initDebugLayerCallback();
    bool initSurface(Window* pWindow);
    bool pickPhysicalDevice();
    bool pickQueueFamilyIndices();
    bool initLogicalDevice();
    bool initSwapchain(const Window* pWindow);
    bool initSwapchainImageViews();

    bool verifyRequiredExtensionsSupported(const std::vector<std::string>& extensionNames);
    bool verifyRequiredDeviceExtensionsSupported(VkPhysicalDevice physicalDevice);
    bool verifyLayerSupport();
    bool querySwapchainSupport(VkPhysicalDevice physicalDevice, SwapchainSupportDetails& supportDetails) const;
    void chooseSwapchainFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
    VkPresentModeKHR chooseSwapchainPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) const;
    VkExtent2D chooseSwapchainExtent(const VkSurfaceCapabilitiesKHR& capabilities, const Window* pWindow) const;

private:
    VkInstance m_Instance;
    VkPhysicalDevice m_PhysicalDevice;
    VkDevice m_Device;
    VkSurfaceKHR m_Surface;
    VkSwapchainKHR m_Swapchain;
    std::vector<VkImage> m_SwapchainImages;
    std::vector<VkImageView> m_SwapchainImageViews;

    VkSurfaceFormatKHR m_SwapchainFormat;

    VkDebugUtilsMessengerEXT m_DebugMessenger;

    QueueFamilyIndices m_QueueFamilyIndices;
};
