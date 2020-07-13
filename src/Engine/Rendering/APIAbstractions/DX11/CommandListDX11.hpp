#pragma once

#include <Engine/Rendering/APIAbstractions/ICommandList.hpp>

class DeviceDX11;
class PipelineDX11;
struct ID3D11CommandList;
struct ID3D11DeviceContext;
struct ID3D11Resource;

class CommandListDX11 : public ICommandList
{
public:
    static CommandListDX11* create(DeviceDX11* pDevice);

public:
    CommandListDX11(ID3D11DeviceContext* pContext, DeviceDX11* pDevice);
    ~CommandListDX11() override final;

    ID3D11DeviceContext* getContext() { return m_pContext; }

    bool begin(COMMAND_LIST_USAGE usageFlags, CommandListBeginInfo* pBeginInfo) override final;
    bool reset() override final;
    bool end() override final;

    void executeSecondaryCommandList(ICommandList* pSecondaryCommandList) override final;

    void beginRenderPass(IRenderPass* pRenderPass, const RenderPassBeginInfo& beginInfo) override final;
    void nextSubpass(COMMAND_LIST_LEVEL subpassCommandListLevel) override final {};
    void endRenderPass() override final;
    void bindPipeline(IPipeline* pPipeline) override final;

    // Shader resources
    void bindDescriptorSet(DescriptorSet* pDescriptorSet, IPipelineLayout* pPipelineLayout) override final;

    void bindVertexBuffer(uint32_t firstBinding, IBuffer* pBuffer) override final;
    void bindIndexBuffer(IBuffer* pBuffer) override final;

    // Rasterizer
    void bindViewport(const Viewport* pViewport) override final;
    void bindScissor(const Rectangle2D& scissorRectangle) override final;

    void draw(size_t vertexCount) override final;
    void drawIndexed(size_t indexCount) override final;

    void convertTextureLayout(TEXTURE_LAYOUT oldLayout, TEXTURE_LAYOUT newLayout, Texture* pTexture, PIPELINE_STAGE srcStage, PIPELINE_STAGE dstStage) override final {};

    void copyBuffer(IBuffer* pSrc, IBuffer* pDst, size_t byteSize) override final;
    void copyBufferToTexture(IBuffer* pBuffer, Texture* pTexture, uint32_t width, uint32_t height) override final;

    inline ID3D11CommandList* getCommandList() { return m_pCommandList; }

private:
    void copyResource(ID3D11Resource* pSrc, ID3D11Resource* pDst, UINT width, UINT height);

private:
    // Deferred context
    ID3D11DeviceContext* m_pContext;
    ID3D11CommandList* m_pCommandList;

    DeviceDX11* m_pDevice;

    PipelineDX11* m_pBoundPipeline;
};
