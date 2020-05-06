#pragma once

#include <Engine/Rendering/APIAbstractions/ICommandList.hpp>

struct ID3D11Device;
struct ID3D11DeviceContext;

class CommandListDX11 : public ICommandList
{
public:
    CommandListDX11(ID3D11DeviceContext* pImmediateContext, ID3D11Device* pDevice);
    ~CommandListDX11() override final;

    ID3D11DeviceContext* getContext() { return m_pContext; }

    void execute() override final;

    void map(IBuffer* pBuffer, void** ppMappedMemory);
    void unmap(IBuffer* pBuffer);
    void bindBuffer(int slot, SHADER_TYPE shaderStages, IBuffer* pBuffer) override final;
    void bindVertexBuffer(int slot, size_t vertexSize, IBuffer* pBuffer) override final;
    void bindIndexBuffer(IBuffer* pBuffer) override final;

    void bindShaderResourceTexture(int slot, SHADER_TYPE shaderStages, Texture* pTexture) override final;
    void bindRenderTarget(Texture* pRenderTarget, Texture* pDepthStencil) override final;

    void bindShaders(const Program* program) override final;

    void bindRasterizerState(IRasterizerState* pRasterizerState) override final;

    void draw(size_t vertexCount) override final;
    void drawIndexed(size_t indexCount) override final;

    void convertTextureLayout(TEXTURE_LAYOUT oldLayout, TEXTURE_LAYOUT newLayout, Texture* pTexture) override final;

private:
    // Deferred context
    ID3D11DeviceContext* m_pContext;
    ID3D11DeviceContext* m_pImmediateContext;
};
