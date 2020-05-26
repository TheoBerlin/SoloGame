#pragma once

#include <Engine/Rendering/APIAbstractions/ICommandList.hpp>

class PipelineDX11;
struct ID3D11Device;
struct ID3D11DeviceContext;

class CommandListDX11 : public ICommandList
{
public:
    CommandListDX11(ID3D11DeviceContext* pImmediateContext, ID3D11Device* pDevice);
    ~CommandListDX11() override final;

    ID3D11DeviceContext* getContext() { return m_pContext; }

    void execute() override final;

    void beginRenderPass(IRenderPass* pRenderPass, const RenderPassBeginInfo& beginInfo) override final;
    void bindPipeline(IPipeline* pPipeline) override final;

    // Shader resources
    void bindDescriptorSet(DescriptorSet* pDescriptorSet) override final;

    void map(IBuffer* pBuffer, void** ppMappedMemory);
    void unmap(IBuffer* pBuffer);

    void bindVertexBuffer(uint32_t firstBinding, IBuffer* pBuffer) override final;
    void bindIndexBuffer(IBuffer* pBuffer) override final;

    // Rasterizer
    void bindViewport(const Viewport* pViewport) override final;

    void draw(size_t vertexCount) override final;
    void drawIndexed(size_t indexCount) override final;

    void convertTextureLayout(TEXTURE_LAYOUT oldLayout, TEXTURE_LAYOUT newLayout, Texture* pTexture) override final;

private:
    // Deferred context
    ID3D11DeviceContext* m_pContext;
    ID3D11DeviceContext* m_pImmediateContext;

    PipelineDX11* m_pBoundPipeline;
};
