#pragma once

#include <Engine/Rendering/APIAbstractions/InputLayout.hpp>
#include <Engine/Rendering/APIAbstractions/Shader.hpp>
#include <Engine/Rendering/APIAbstractions/Texture.hpp>

class BlendState;
class IBuffer;
class IDepthStencilState;
class DescriptorSet;
class IPipeline;
class IRasterizerState;
class IRenderPass;
class ISampler;
struct Program;
struct RenderPassBeginInfo;
struct Viewport;

class ICommandList
{
public:
    virtual ~ICommandList() = 0 {};

    virtual void execute() = 0;

    virtual void beginRenderPass(IRenderPass* pRenderPass, const RenderPassBeginInfo& beginInfo) = 0;
    virtual void bindPipeline(IPipeline* pPipeline) = 0;

    // Shader resources
    virtual void bindDescriptorSet(DescriptorSet* pDescriptorSet) = 0;

    virtual void map(IBuffer* pBuffer, void** ppMappedMemory) = 0;
    virtual void unmap(IBuffer* pBuffer) = 0;

    virtual void bindVertexBuffer(uint32_t firstBinding, IBuffer* pBuffer) = 0;
    virtual void bindIndexBuffer(IBuffer* pBuffer) = 0;

    // Rasterizer
    virtual void bindViewport(const Viewport* pViewport) = 0;

    virtual void draw(size_t vertexCount) = 0;
    virtual void drawIndexed(size_t indexCount) = 0;

    virtual void convertTextureLayout(TEXTURE_LAYOUT oldLayout, TEXTURE_LAYOUT newLayout, Texture* pTexture) = 0;
};
