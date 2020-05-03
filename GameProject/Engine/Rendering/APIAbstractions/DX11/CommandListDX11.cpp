#include "CommandListDX11.hpp"

#include <Engine/Rendering/APIAbstractions/DX11/BufferDX11.hpp>
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

void CommandListDX11::bindVertexBuffer(int slot, size_t vertexSize, IBuffer* pBuffer)
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
    m_pContext->VSSetShader(program->vertexShader, nullptr, 0);
    m_pContext->HSSetShader(program->hullShader, nullptr, 0);
    m_pContext->DSSetShader(program->domainShader, nullptr, 0);
    m_pContext->GSSetShader(program->geometryShader, nullptr, 0);
    m_pContext->PSSetShader(program->pixelShader, nullptr, 0);
}

void CommandListDX11::draw(size_t vertexCount)
{
    m_pContext->Draw((UINT)vertexCount, 0);
}

void CommandListDX11::drawIndexed(size_t indexCount)
{
    m_pContext->DrawIndexed((UINT)indexCount, 0, 0);
}
