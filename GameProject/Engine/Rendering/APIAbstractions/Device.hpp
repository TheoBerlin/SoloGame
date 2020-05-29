#pragma once

#include <Engine/Rendering/APIAbstractions/DescriptorPoolHandler.hpp>
#include <Engine/Rendering/APIAbstractions/Shader.hpp>
#include <Engine/Rendering/ShaderHandler.hpp>

#define NOMINMAX
#include <DirectXMath.h>

#include <string>

struct SwapchainInfo {
    uint32_t FrameRateLimit;
    uint32_t Multisamples;
    bool Windowed;
};

class BlendState;
class IBuffer;
class ICommandList;
class IDepthStencilState;
class IFramebuffer;
class InputLayout;
class IPipeline;
class IPipelineLayout;
class IRasterizerState;
class IRenderPass;
class ISampler;
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
struct TextureInfo;

enum class RENDERING_API {
    DIRECTX11,
    VULKAN
};

class Device
{
public:
    static Device* create(RENDERING_API API);

public:
    Device();
    virtual ~Device();

    virtual bool init(const SwapchainInfo& swapChainInfo, Window* pWindow) = 0;
    bool finalize(const DescriptorCounts& descriptorCounts);

    virtual void presentBackBuffer() = 0;

    virtual ICommandList* createCommandList() = 0;

    virtual IDescriptorSetLayout* createDescriptorSetLayout() = 0;
    DescriptorSet* allocateDescriptorSet(const IDescriptorSetLayout* pDescriptorSetLayout);

    virtual IFramebuffer* createFramebuffer(const FramebufferInfo& framebufferInfo) = 0;
    virtual IRenderPass* createRenderPass(const RenderPassInfo& renderPassInfo) = 0;

    virtual IPipelineLayout* createPipelineLayout(std::vector<IDescriptorSetLayout*> descriptorSetLayout) = 0;
    virtual IPipeline* createPipeline(const PipelineInfo& pipelineInfo) = 0;

    // Shader resources
    virtual IBuffer* createBuffer(const BufferInfo& bufferInfo) = 0;
    virtual IBuffer* createVertexBuffer(const void* pVertices, size_t vertexSize, size_t vertexCount) = 0;
    virtual IBuffer* createIndexBuffer(const unsigned* pIndices, size_t indexCount) = 0;

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

    Texture* getBackBuffer()            { return m_pBackBuffer; }
    Texture* getDepthStencil()          { return m_pDepthTexture; }
    ShaderHandler* getShaderHandler()   { return m_pShaderHandler; }

protected:
    friend DescriptorPoolHandler;

    virtual DescriptorPool* createDescriptorPool(const DescriptorCounts& poolSize) = 0;

protected:
    DescriptorPoolHandler m_DescriptorPoolHandler;

    Texture* m_pBackBuffer;
    Texture* m_pDepthTexture;

private:
    virtual Shader* compileShader(SHADER_TYPE shaderType, const std::string& filePath, const InputLayoutInfo* pInputLayoutInfo, InputLayout** ppInputLayout) = 0;

private:
    ShaderHandler* m_pShaderHandler;
};
