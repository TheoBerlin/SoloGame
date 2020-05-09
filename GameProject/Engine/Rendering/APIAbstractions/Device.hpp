#pragma once

#include <Engine/Rendering/APIAbstractions/Shader.hpp>

#define NOMINMAX
#include <DirectXMath.h>

#include <string>

struct SwapChainInfo {
    uint32_t FrameRateLimit;
    uint32_t Multisamples;
    bool Windowed;
};

class BlendState;
class IBuffer;
class ICommandList;
class InputLayout;
class IRasterizerState;
class ISampler;
class Texture;
class Window;
struct BlendStateInfo;
struct BufferInfo;
struct InputLayoutInfo;
struct RasterizerStateInfo;
struct SamplerInfo;
struct TextureInfo;

class Device
{
public:
    Device();
    virtual ~Device();

    virtual bool init(const SwapChainInfo& swapChainInfo, Window* pWindow) = 0;

    virtual void clearBackBuffer() = 0;
    virtual void presentBackBuffer() = 0;

    virtual ICommandList* createCommandList() = 0;

    // Shader resources
    virtual IBuffer* createBuffer(const BufferInfo& bufferInfo) = 0;
    virtual IBuffer* createVertexBuffer(const void* pVertices, size_t vertexSize, size_t vertexCount) = 0;
    virtual IBuffer* createIndexBuffer(const unsigned* pIndices, size_t indexCount) = 0;

    virtual Texture* createTextureFromFile(const std::string& filePath) = 0;
    virtual Texture* createTexture(const TextureInfo& textureInfo) = 0;

    virtual ISampler* createSampler(const SamplerInfo& samplerInfo) = 0;

    Shader* createShader(SHADER_TYPE shaderType, const std::string& filePath, const InputLayoutInfo* pInputLayoutInfo = nullptr, InputLayout** ppInputLayout = nullptr);

    // Rasterizer
    virtual IRasterizerState* createRasterizerState(const RasterizerStateInfo& rasterizerInfo) = 0;

    // Output merger
    virtual BlendState* createBlendState(const BlendStateInfo& blendStateInfo) = 0;

    Texture* getBackBuffer()    { return m_pBackBuffer; }
    Texture* getDepthStencil()  { return m_pDepthTexture; }

private:
    virtual Shader* compileShader(SHADER_TYPE shaderType, const std::string& filePath, const InputLayoutInfo* pInputLayoutInfo, InputLayout** ppInputLayout) = 0;
    virtual std::string getShaderPostfixAndExtension(SHADER_TYPE shaderType) = 0;

protected:
    Texture* m_pBackBuffer;
    Texture* m_pDepthTexture;
};
