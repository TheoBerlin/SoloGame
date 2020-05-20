#include "CommandListDX11.hpp"

#include <Engine/Rendering/APIAbstractions/DX11/BlendStateDX11.hpp>
#include <Engine/Rendering/APIAbstractions/DX11/BufferDX11.hpp>
#include <Engine/Rendering/APIAbstractions/DX11/DepthStencilStateDX11.hpp>
#include <Engine/Rendering/APIAbstractions/DX11/DescriptorSetDX11.hpp>
#include <Engine/Rendering/APIAbstractions/DX11/InputLayoutDX11.hpp>
#include <Engine/Rendering/APIAbstractions/DX11/RasterizerStateDX11.hpp>
#include <Engine/Rendering/APIAbstractions/DX11/RenderPassDX11.hpp>
#include <Engine/Rendering/APIAbstractions/DX11/SamplerDX11.hpp>
#include <Engine/Rendering/APIAbstractions/DX11/ShaderDX11.hpp>
#include <Engine/Rendering/APIAbstractions/DX11/TextureDX11.hpp>
#include <Engine/Rendering/ShaderHandler.hpp>
#include <Engine/Utils/DirectXUtils.hpp>
#include <Engine/Utils/Logger.hpp>

#define NOMINMAX
#include <d3d11.h>

CommandListDX11::CommandListDX11(ID3D11DeviceContext* pImmediateContext, ID3D11Device* pDevice)
    :m_pContext(nullptr),
    m_pImmediateContext(pImmediateContext)
{
    HRESULT hr = pDevice->CreateDeferredContext(0, &m_pContext);
    if (FAILED(hr)) {
        LOG_ERROR("Failed to create deferred context: %s", hresultToString(hr).c_str());
    }
}

CommandListDX11::~CommandListDX11()
{
    SAFERELEASE(m_pContext)
}

void CommandListDX11::execute()
{
    ID3D11CommandList* pCommandList = nullptr;
    HRESULT hr = m_pContext->FinishCommandList(FALSE, &pCommandList);
    if (FAILED(hr)) {
        LOG_WARNING("Failed to finish command list: %s", hresultToString(hr).c_str());
    }

    m_pImmediateContext->ExecuteCommandList(pCommandList, FALSE);
    pCommandList->Release();
}

void CommandListDX11::beginRenderPass(IRenderPass* pRenderPass, const RenderPassBeginInfo& beginInfo)
{
    RenderPassDX11* pRenderPassDX = reinterpret_cast<RenderPassDX11*>(pRenderPass);
    pRenderPassDX->begin(beginInfo, m_pContext);
}

void CommandListDX11::bindPrimitiveTopology(PRIMITIVE_TOPOLOGY primitiveTopology)
{
    m_pContext->IASetPrimitiveTopology(convertPrimitiveTopology(primitiveTopology));
}

void CommandListDX11::bindInputLayout(InputLayout* pInputLayout)
{
    ID3D11InputLayout* pInputLayoutDX = reinterpret_cast<InputLayoutDX11*>(pInputLayout)->getInputLayout();
    m_pContext->IASetInputLayout(pInputLayoutDX);
}

void CommandListDX11::bindDescriptorSet(DescriptorSet* pDescriptorSet)
{
    DescriptorSetDX11* pDescriptorSetDX = reinterpret_cast<DescriptorSetDX11*>(pDescriptorSet);
    pDescriptorSetDX->bind(m_pContext);
}

void CommandListDX11::map(IBuffer* pBuffer, void** ppMappedMemory)
{
    ID3D11Buffer* pBufferDX = reinterpret_cast<BufferDX11*>(pBuffer)->getBuffer();

    D3D11_MAPPED_SUBRESOURCE mappedResources = {};
    m_pContext->Map(pBufferDX, 0u, D3D11_MAP_WRITE_DISCARD, 0u, &mappedResources);
    (*ppMappedMemory) = mappedResources.pData;
}

void CommandListDX11::unmap(IBuffer* pBuffer)
{
    m_pContext->Unmap(reinterpret_cast<BufferDX11*>(pBuffer)->getBuffer(), 0u);
}

void CommandListDX11::bindVertexBuffer(int slot, uint32_t vertexSize, IBuffer* pBuffer)
{
    ID3D11Buffer* pBufferDX = reinterpret_cast<BufferDX11*>(pBuffer)->getBuffer();
    UINT uVertexSize = (UINT)vertexSize;
    UINT offsets = 0u;

    m_pContext->IASetVertexBuffers(0u, 1u, &pBufferDX, &uVertexSize, &offsets);
}

void CommandListDX11::bindIndexBuffer(IBuffer* pBuffer)
{
    ID3D11Buffer* pBufferDX = reinterpret_cast<BufferDX11*>(pBuffer)->getBuffer();

    m_pContext->IASetIndexBuffer(pBufferDX, DXGI_FORMAT_R32_UINT, 0);
}

void CommandListDX11::bindShaders(const Program* program)
{
    ID3D11VertexShader* pVertexShader       = program->pVertexShader ? reinterpret_cast<ShaderDX11*>(program->pVertexShader)->getVertexShader() : nullptr;
    ID3D11HullShader* pHullShader           = program->pHullShader ? reinterpret_cast<ShaderDX11*>(program->pHullShader)->getHullShader() : nullptr;
    ID3D11DomainShader* pDomainShader       = program->pDomainShader ? reinterpret_cast<ShaderDX11*>(program->pDomainShader)->getDomainShader() : nullptr;
    ID3D11GeometryShader* pGeometryShader   = program->pGeometryShader ? reinterpret_cast<ShaderDX11*>(program->pGeometryShader)->getGeometryShader() : nullptr;
    ID3D11PixelShader* pFragmentShader      = program->pFragmentShader ? reinterpret_cast<ShaderDX11*>(program->pFragmentShader)->getFragmentShader() : nullptr;

    m_pContext->VSSetShader(pVertexShader, nullptr, 0);
    m_pContext->HSSetShader(pHullShader, nullptr, 0);
    m_pContext->DSSetShader(pDomainShader, nullptr, 0);
    m_pContext->GSSetShader(pGeometryShader, nullptr, 0);
    m_pContext->PSSetShader(pFragmentShader, nullptr, 0);
}

void CommandListDX11::bindRasterizerState(IRasterizerState* pRasterizerState)
{
    ID3D11RasterizerState* pRasterizerStateDX = reinterpret_cast<RasterizerStateDX11*>(pRasterizerState)->getRasterizerState();
    m_pContext->RSSetState(pRasterizerStateDX);
}

void CommandListDX11::bindViewport(const Viewport* pViewport)
{
    m_pContext->RSSetViewports(1u, (const D3D11_VIEWPORT*)pViewport);
}

void CommandListDX11::bindBlendState(BlendState* pBlendState)
{
    ID3D11BlendState* pBlendStateDX = reinterpret_cast<BlendStateDX11*>(pBlendState)->getBlendState();
    m_pContext->OMSetBlendState(pBlendStateDX, (FLOAT*)pBlendState->getBlendConstants(), D3D11_COLOR_WRITE_ENABLE_ALL);
}

void CommandListDX11::bindDepthStencilState(IDepthStencilState* pDepthStencilState)
{
    DepthStencilStateDX11* pDepthStencilStateDX = reinterpret_cast<DepthStencilStateDX11*>(pDepthStencilState);
    m_pContext->OMSetDepthStencilState(pDepthStencilStateDX->getDepthStencilState(), pDepthStencilStateDX->getStencilReference());
}

void CommandListDX11::draw(size_t vertexCount)
{
    m_pContext->Draw((UINT)vertexCount, 0);
}

void CommandListDX11::drawIndexed(size_t indexCount)
{
    m_pContext->DrawIndexed((UINT)indexCount, 0, 0);
}

void CommandListDX11::convertTextureLayout(TEXTURE_LAYOUT oldLayout, TEXTURE_LAYOUT newLayout, Texture* pTexture)
{
    pTexture->convertTextureLayout(oldLayout, newLayout);
}
