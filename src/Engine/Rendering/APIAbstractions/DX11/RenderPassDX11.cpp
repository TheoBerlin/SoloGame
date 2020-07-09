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

    std::vector<RenderTargetInfo>& renderTargetInfos = pFramebuffer->getRenderTargets();
    size_t depthStencilIdx = pFramebuffer->getDepthStencilIdx();
    bool clearDepthStencil = false;

    for (size_t clearIndex : m_ClearIndices) {
        const ClearValue& clearValue = beginInfo.pClearValues[clearIndex];

        if (clearIndex != depthStencilIdx) {
            switch (renderTargetInfos[clearIndex].FormatType) {
                case FORMAT_PRIMITIVE_TYPE::INTEGER:
                    pClearColor[0] = (float)clearValue.ClearColorValue.int32[0] / 255.0f;
                    pClearColor[1] = (float)clearValue.ClearColorValue.int32[1] / 255.0f;
                    pClearColor[2] = (float)clearValue.ClearColorValue.int32[2] / 255.0f;
                    pClearColor[3] = (float)clearValue.ClearColorValue.int32[3] / 255.0f;
                    break;
                case FORMAT_PRIMITIVE_TYPE::UNSIGNED_INTEGER:
                    pClearColor[0] = (float)clearValue.ClearColorValue.uint32[0] / 255.0f;
                    pClearColor[1] = (float)clearValue.ClearColorValue.uint32[1] / 255.0f;
                    pClearColor[2] = (float)clearValue.ClearColorValue.uint32[2] / 255.0f;
                    pClearColor[3] = (float)clearValue.ClearColorValue.uint32[3] / 255.0f;
                    break;
                case FORMAT_PRIMITIVE_TYPE::FLOAT:
                    std::memcpy(pClearColor, clearValue.ClearColorValue.float32, sizeof(float) * 4u);
                    break;
            }

            pContext->ClearRenderTargetView(renderTargetInfos[clearIndex].pRTV, pClearColor);
        } else {
            pContext->ClearDepthStencilView(pFramebuffer->getDepthStencil(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, clearValue.DepthStencilValue.Depth, clearValue.DepthStencilValue.Stencil);
        }
    }

    std::vector<ID3D11RenderTargetView*> renderTargetViews;
    renderTargetViews.reserve(renderTargetInfos.size());
    for (const RenderTargetInfo& renderTargetInfo : renderTargetInfos) {
        renderTargetViews.push_back(renderTargetInfo.pRTV);
    }

    pContext->OMSetRenderTargets((UINT)renderTargetViews.size(), renderTargetViews.data(), pFramebuffer->getDepthStencil());
}
