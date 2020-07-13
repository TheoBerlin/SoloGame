#pragma once

#include <Engine/Rendering/APIAbstractions/InputLayout.hpp>
#include <Engine/Rendering/APIAbstractions/Shader.hpp>
#include <Engine/Rendering/APIAbstractions/Texture.hpp>
#include <Engine/Utils/EnumClass.hpp>

class BlendState;
class IBuffer;
class IDepthStencilState;
class DescriptorSet;
class IPipeline;
class IRasterizerState;
class IRenderPass;
class ISampler;
struct RenderPassBeginInfo;
struct Viewport;

// Needed to begin recording on secondary command lists
struct CommandListBeginInfo {
    IRenderPass* pRenderPass;
    uint32_t Subpass;
    Framebuffer* pFramebuffer; // Optional, but may improve performance if specified
};

enum class COMMAND_LIST_USAGE : uint32_t {
    ONE_TIME_SUBMIT     = 1,
    WITHIN_RENDER_PASS  = ONE_TIME_SUBMIT << 1,
    SIMULTANEOUS_USE    = WITHIN_RENDER_PASS << 1
};

DEFINE_BITMASK_OPERATIONS(COMMAND_LIST_USAGE)

class ICommandList
{
public:
    virtual ~ICommandList() = 0 {};

    // Implicitly resets the command list
    virtual bool begin(COMMAND_LIST_USAGE usageFlags, CommandListBeginInfo* pBeginInfo) = 0;
    virtual bool reset() = 0;
    virtual bool end() = 0;

    virtual void executeSecondaryCommandList(ICommandList* pSecondaryCommandList) = 0;

    virtual void beginRenderPass(IRenderPass* pRenderPass, const RenderPassBeginInfo& beginInfo) = 0;
    virtual void nextSubpass(COMMAND_LIST_LEVEL subpassCommandListLevel) = 0;
    virtual void endRenderPass() = 0;
    virtual void bindPipeline(IPipeline* pPipeline) = 0;

    // Shader resources
    virtual void bindDescriptorSet(DescriptorSet* pDescriptorSet, IPipelineLayout* pPipelineLayout) = 0;

    virtual void bindVertexBuffer(uint32_t firstBinding, IBuffer* pBuffer) = 0;
    virtual void bindIndexBuffer(IBuffer* pBuffer) = 0;

    // Rasterizer
    virtual void bindViewport(const Viewport* pViewport) = 0;
    virtual void bindScissor(const Rectangle2D& scissorRectangle) = 0;

    virtual void draw(size_t vertexCount) = 0;
    virtual void drawIndexed(size_t indexCount) = 0;

    virtual void convertTextureLayout(TEXTURE_LAYOUT oldLayout, TEXTURE_LAYOUT newLayout, Texture* pTexture, PIPELINE_STAGE srcStage, PIPELINE_STAGE dstStage) = 0;

    virtual void copyBuffer(IBuffer* pSrc, IBuffer* pDst, size_t byteSize) = 0;
    virtual void copyBufferToTexture(IBuffer* pBuffer, Texture* pTexture, uint32_t width, uint32_t height) = 0;
};
