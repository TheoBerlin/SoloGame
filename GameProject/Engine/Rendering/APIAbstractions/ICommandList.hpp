#pragma once

#include <Engine/Rendering/APIAbstractions/InputLayout.hpp>
#include <Engine/Rendering/APIAbstractions/Shader.hpp>
#include <Engine/Rendering/APIAbstractions/Texture.hpp>

class BlendState;
class IBuffer;
class IDepthStencilState;
class IRasterizerState;
class ISampler;
struct Program;
struct Viewport;

class ICommandList
{
public:
    virtual ~ICommandList() = 0 {};

    virtual void execute() = 0;

    // Input assember
    virtual void bindPrimitiveTopology(PRIMITIVE_TOPOLOGY primitiveTopology) = 0;
    virtual void bindInputLayout(InputLayout* pInputLayout) = 0;

    // Shader resources
    virtual void map(IBuffer* pBuffer, void** ppMappedMemory) = 0;
    virtual void unmap(IBuffer* pBuffer) = 0;
    virtual void bindBuffer(int slot, SHADER_TYPE shaderStages, IBuffer* pBuffer) = 0;
    virtual void bindVertexBuffer(int slot, uint32_t vertexSize, IBuffer* pBuffer) = 0;
    virtual void bindIndexBuffer(IBuffer* pBuffer) = 0;

    virtual void bindShaderResourceTexture(int slot, SHADER_TYPE shaderStages, Texture* pTexture) = 0;

    virtual void bindSampler(uint32_t slot, SHADER_TYPE shaderStages, ISampler* pSampler) = 0;
    virtual void bindShaders(const Program* program) = 0;

    // Rasterizer
    virtual void bindRasterizerState(IRasterizerState* pRasterizerState) = 0;
    virtual void bindViewport(const Viewport* pViewport) = 0;

    // Output merger
    virtual void bindRenderTarget(Texture* pRenderTarget, Texture* pDepthStencil) = 0;
    virtual void bindBlendState(BlendState* pBlendState) = 0;
    virtual void bindDepthStencilState(IDepthStencilState* pDepthStencilState) = 0;

    virtual void draw(size_t vertexCount) = 0;
    virtual void drawIndexed(size_t indexCount) = 0;

    virtual void convertTextureLayout(TEXTURE_LAYOUT oldLayout, TEXTURE_LAYOUT newLayout, Texture* pTexture) = 0;
};
