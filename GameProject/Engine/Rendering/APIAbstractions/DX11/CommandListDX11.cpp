#include "CommandListDX11.hpp"

#include <Engine/Rendering/APIAbstractions/DX11/BlendStateDX11.hpp>
#include <Engine/Rendering/APIAbstractions/DX11/BufferDX11.hpp>
#include <Engine/Rendering/APIAbstractions/DX11/InputLayoutDX11.hpp>
#include <Engine/Rendering/APIAbstractions/DX11/RasterizerStateDX11.hpp>
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

void CommandListDX11::bindInputLayout(InputLayout* pInputLayout)
{
    ID3D11InputLayout* pInputLayoutDX = reinterpret_cast<InputLayoutDX11*>(pInputLayout)->getInputLayout();
    m_pContext->IASetInputLayout(pInputLayoutDX);
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

void CommandListDX11::bindBuffer(int slot, SHADER_TYPE shaderStages, IBuffer* pBuffer)
{
    ID3D11Buffer* pBufferDX = reinterpret_cast<BufferDX11*>(pBuffer)->getBuffer();

    UINT uSlot = (UINT)slot;

    if (HAS_FLAG(shaderStages, SHADER_TYPE::VERTEX_SHADER)) {
        m_pContext->VSSetConstantBuffers(uSlot, 1, &pBufferDX);
    }

    if (HAS_FLAG(shaderStages, SHADER_TYPE::HULL_SHADER)) {
        m_pContext->HSSetConstantBuffers(uSlot, 1, &pBufferDX);
    }

    if (HAS_FLAG(shaderStages, SHADER_TYPE::DOMAIN_SHADER)) {
        m_pContext->DSSetConstantBuffers(uSlot, 1, &pBufferDX);
    }

    if (HAS_FLAG(shaderStages, SHADER_TYPE::GEOMETRY_SHADER)) {
        m_pContext->GSSetConstantBuffers(uSlot, 1, &pBufferDX);
    }

    if (HAS_FLAG(shaderStages, SHADER_TYPE::FRAGMENT_SHADER)) {
        m_pContext->PSSetConstantBuffers(uSlot, 1, &pBufferDX);
    }
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

void CommandListDX11::bindShaderResourceTexture(int slot, SHADER_TYPE shaderStages, Texture* pTexture)
{
    ID3D11ShaderResourceView* pSRV = reinterpret_cast<TextureDX11*>(pTexture)->getSRV();

    UINT uSlot = (UINT)slot;

    if (HAS_FLAG(shaderStages, SHADER_TYPE::VERTEX_SHADER)) {
        m_pContext->VSSetShaderResources(uSlot, 1, &pSRV);
    }

    if (HAS_FLAG(shaderStages, SHADER_TYPE::HULL_SHADER)) {
        m_pContext->HSSetShaderResources(uSlot, 1, &pSRV);
    }

    if (HAS_FLAG(shaderStages, SHADER_TYPE::DOMAIN_SHADER)) {
        m_pContext->DSSetShaderResources(uSlot, 1, &pSRV);
    }

    if (HAS_FLAG(shaderStages, SHADER_TYPE::GEOMETRY_SHADER)) {
        m_pContext->GSSetShaderResources(uSlot, 1, &pSRV);
    }

    if (HAS_FLAG(shaderStages, SHADER_TYPE::FRAGMENT_SHADER)) {
        m_pContext->PSSetShaderResources(uSlot, 1, &pSRV);
    }
}

void CommandListDX11::bindSampler(uint32_t slot, SHADER_TYPE shaderStages, ISampler* pSampler)
{
    ID3D11SamplerState* pSamplerDX = reinterpret_cast<SamplerDX11*>(pSampler)->getSamplerState();

    UINT uSlot = (UINT)slot;

    if (HAS_FLAG(shaderStages, SHADER_TYPE::VERTEX_SHADER)) {
        m_pContext->VSSetSamplers(uSlot, 1, &pSamplerDX);
    }

    if (HAS_FLAG(shaderStages, SHADER_TYPE::HULL_SHADER)) {
        m_pContext->HSSetSamplers(uSlot, 1, &pSamplerDX);
    }

    if (HAS_FLAG(shaderStages, SHADER_TYPE::DOMAIN_SHADER)) {
        m_pContext->DSSetSamplers(uSlot, 1, &pSamplerDX);
    }

    if (HAS_FLAG(shaderStages, SHADER_TYPE::GEOMETRY_SHADER)) {
        m_pContext->GSSetSamplers(uSlot, 1, &pSamplerDX);
    }

    if (HAS_FLAG(shaderStages, SHADER_TYPE::FRAGMENT_SHADER)) {
        m_pContext->PSSetSamplers(uSlot, 1, &pSamplerDX);
    }
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

void CommandListDX11::bindRenderTarget(Texture* pRenderTarget, Texture* pDepthStencil)
{
    TextureDX11* pRenderTargetDX = reinterpret_cast<TextureDX11*>(pRenderTarget);
    TextureDX11* pDepthStencilDX = reinterpret_cast<TextureDX11*>(pDepthStencil);

    ID3D11RenderTargetView* pRTV = pRenderTargetDX ? pRenderTargetDX->getRTV() : nullptr;
    ID3D11DepthStencilView* pDSV = pDepthStencilDX ? pDepthStencilDX->getDSV() : nullptr;

    m_pContext->OMSetRenderTargets(1, &pRTV, pDSV);
}

void CommandListDX11::bindBlendState(BlendState* pBlendState)
{
    ID3D11BlendState* pBlendStateDX = reinterpret_cast<BlendStateDX11*>(pBlendState)->getBlendState();
    m_pContext->OMSetBlendState(pBlendStateDX, (FLOAT*)pBlendState->getBlendConstants(), D3D11_COLOR_WRITE_ENABLE_ALL);
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
