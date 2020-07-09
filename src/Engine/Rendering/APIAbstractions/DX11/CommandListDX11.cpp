#include "CommandListDX11.hpp"

#include <Engine/Rendering/APIAbstractions/DX11/BlendStateDX11.hpp>
#include <Engine/Rendering/APIAbstractions/DX11/BufferDX11.hpp>
#include <Engine/Rendering/APIAbstractions/DX11/DepthStencilStateDX11.hpp>
#include <Engine/Rendering/APIAbstractions/DX11/DescriptorSetDX11.hpp>
#include <Engine/Rendering/APIAbstractions/DX11/DeviceDX11.hpp>
#include <Engine/Rendering/APIAbstractions/DX11/InputLayoutDX11.hpp>
#include <Engine/Rendering/APIAbstractions/DX11/PipelineDX11.hpp>
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

CommandListDX11* CommandListDX11::create(DeviceDX11* pDevice)
{
    ID3D11DeviceContext* pContext = nullptr;
    HRESULT hr = pDevice->getDevice()->CreateDeferredContext(0, &pContext);
    if (FAILED(hr)) {
        LOG_ERROR("Failed to create deferred context: %s", hresultToString(hr).c_str());
        return nullptr;
    }

    return DBG_NEW CommandListDX11(pContext, pDevice);
}

CommandListDX11::CommandListDX11(ID3D11DeviceContext* pContext, DeviceDX11* pDevice)
    :m_pContext(pContext),
    m_pCommandList(nullptr),
    m_pDevice(pDevice),
    m_pBoundPipeline(nullptr)
{}

CommandListDX11::~CommandListDX11()
{
    SAFERELEASE(m_pCommandList)
    SAFERELEASE(m_pContext)
}

bool CommandListDX11::begin(COMMAND_LIST_USAGE usageFlags, CommandListBeginInfo* pBeginInfo)
{
    SAFERELEASE(m_pCommandList)
    m_pCommandList = nullptr;
    return true;
}

bool CommandListDX11::reset()
{
    SAFERELEASE(m_pCommandList)
    m_pCommandList = nullptr;
    return true;
}

bool CommandListDX11::end()
{
    HRESULT hr = m_pContext->FinishCommandList(FALSE, &m_pCommandList);
    if (FAILED(hr)) {
        LOG_WARNING("Failed to finish command list: %s", hresultToString(hr).c_str());
        return false;
    }

    return true;
}

void CommandListDX11::beginRenderPass(IRenderPass* pRenderPass, const RenderPassBeginInfo& beginInfo)
{
    RenderPassDX11* pRenderPassDX = reinterpret_cast<RenderPassDX11*>(pRenderPass);
    pRenderPassDX->begin(beginInfo, m_pContext);
}

void CommandListDX11::endRenderPass()
{
    // Unbind any render targets
    m_pContext->OMSetRenderTargets(0u, nullptr, nullptr);
}

void CommandListDX11::bindPipeline(IPipeline* pPipeline)
{
    PipelineDX11* pPipelineDX = reinterpret_cast<PipelineDX11*>(pPipeline);
    pPipelineDX->bind(m_pContext);
    m_pBoundPipeline = pPipelineDX;
}

void CommandListDX11::bindDescriptorSet(DescriptorSet* pDescriptorSet)
{
    DescriptorSetDX11* pDescriptorSetDX = reinterpret_cast<DescriptorSetDX11*>(pDescriptorSet);
    pDescriptorSetDX->bind(m_pContext);
}

void CommandListDX11::bindVertexBuffer(uint32_t firstBinding, IBuffer* pBuffer)
{
    ID3D11Buffer* pBufferDX = reinterpret_cast<BufferDX11*>(pBuffer)->getBuffer();
    UINT vertexSize         = m_pBoundPipeline->getVertexSize();
    UINT offsets            = 0u;

    m_pContext->IASetVertexBuffers(0u, 1u, &pBufferDX, &vertexSize, &offsets);
}

void CommandListDX11::bindIndexBuffer(IBuffer* pBuffer)
{
    ID3D11Buffer* pBufferDX = reinterpret_cast<BufferDX11*>(pBuffer)->getBuffer();

    m_pContext->IASetIndexBuffer(pBufferDX, DXGI_FORMAT_R32_UINT, 0);
}

void CommandListDX11::bindViewport(const Viewport* pViewport)
{
    m_pContext->RSSetViewports(1u, (const D3D11_VIEWPORT*)pViewport);
}

void CommandListDX11::draw(size_t vertexCount)
{
    m_pContext->Draw((UINT)vertexCount, 0);
}

void CommandListDX11::drawIndexed(size_t indexCount)
{
    m_pContext->DrawIndexed((UINT)indexCount, 0, 0);
}

void CommandListDX11::copyBuffer(IBuffer* pSrc, IBuffer* pDst, size_t byteSize)
{
    BufferDX11* pSrcDX = reinterpret_cast<BufferDX11*>(pSrc);
    BufferDX11* pDstDX = reinterpret_cast<BufferDX11*>(pDst);

    copyResource(pSrcDX->getBuffer(), pDstDX->getBuffer(), (UINT)byteSize, 0u);
}

void CommandListDX11::copyBufferToTexture(IBuffer* pBuffer, Texture* pTexture, uint32_t width, uint32_t height)
{
    ID3D11Resource* pBufferResource = reinterpret_cast<BufferDX11*>(pBuffer)->getBuffer();
    ID3D11Resource* pTextureResource = reinterpret_cast<TextureDX11*>(pTexture)->getResource();

    copyResource(pBufferResource, pTextureResource, (UINT)width, (UINT)height);
}

void CommandListDX11::copyResource(ID3D11Resource* pSrc, ID3D11Resource* pDst, UINT width, UINT height)
{
    D3D11_BOX srcBox = {};
    srcBox.right    = width;
    srcBox.bottom   = height;

    m_pContext->CopySubresourceRegion(pDst, 0u, 0u, 0u, 0u, pSrc, 0u, &srcBox);
}
