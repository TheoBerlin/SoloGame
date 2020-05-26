#include "RenderPassDX11.hpp"

#include <Engine/Rendering/APIAbstractions/DX11/FramebufferDX11.hpp>
#include <Engine/Utils/Debug.hpp>

RenderPassDX11* RenderPassDX11::create(const RenderPassInfo& renderPassInfo)
{
    std::vector<size_t> clearIndices;

    for (size_t attachmentIdx = 0; attachmentIdx < renderPassInfo.AttachmentInfos.size(); attachmentIdx += 1) {
        const AttachmentInfo& attachmentInfo = renderPassInfo.AttachmentInfos[attachmentIdx];

        if (attachmentInfo.LoadOp == ATTACHMENT_LOAD_OP::CLEAR) {
            clearIndices.push_back(attachmentIdx);
        }
    }

    return DBG_NEW RenderPassDX11(clearIndices);
}

RenderPassDX11::RenderPassDX11(std::vector<size_t> clearIndices)
    :m_ClearIndices(clearIndices)
{
    m_ClearIndices.shrink_to_fit();
}

void RenderPassDX11::begin(const RenderPassBeginInfo& beginInfo, ID3D11DeviceContext* pContext)
{
    FramebufferDX11* pFramebuffer = reinterpret_cast<FramebufferDX11*>(beginInfo.pFramebuffer);
    FLOAT pClearColor[4];

    std::vector<ID3D11RenderTargetView*>& renderTargets = pFramebuffer->getRenderTargets();
    size_t depthStencilIdx = pFramebuffer->getDepthStencilIdx();
    bool clearDepthStencil = false;

    for (size_t clearIndex : m_ClearIndices) {
        if (clearIndex == depthStencilIdx) {
            clearDepthStencil = true;
            continue;
        }

        std::memcpy(pClearColor, &beginInfo.ClearColors[clearIndex], sizeof(float) * 4);

        pContext->ClearRenderTargetView(renderTargets[clearIndex], pClearColor);
    }

    if (clearDepthStencil) {
        float depthClearValue = beginInfo.ClearDepthStencilValue.Depth;
        uint32_t stencilClearValue = beginInfo.ClearDepthStencilValue.Stencil;
        pContext->ClearDepthStencilView(pFramebuffer->getDepthStencil(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, depthClearValue, stencilClearValue);
    }

    pContext->OMSetRenderTargets((UINT)renderTargets.size(), renderTargets.data(), pFramebuffer->getDepthStencil());
}
