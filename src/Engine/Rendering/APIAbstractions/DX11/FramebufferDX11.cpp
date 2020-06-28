#include "FramebufferDX11.hpp"

#include <Engine/Rendering/APIAbstractions/DX11/TextureDX11.hpp>
#include <Engine/Utils/Debug.hpp>
#include <Engine/Utils/Logger.hpp>

FramebufferDX11* FramebufferDX11::create(const FramebufferInfo& framebufferInfo)
{
    std::vector<ID3D11RenderTargetView*> renderTargets;
    // Separate the depth stencil from the render targets
    ID3D11DepthStencilView* pDSV = nullptr;
    size_t depthStencilIdx = 0;

    for (size_t textureIdx = 0; textureIdx < framebufferInfo.Attachments.size(); textureIdx += 1) {
        const Texture* pTexture = framebufferInfo.Attachments[textureIdx];

        const TextureDX11* pTextureDX = reinterpret_cast<const TextureDX11*>(pTexture);
        if (!pTextureDX->getDSV()) {
            if (!pTextureDX->getRTV()) {
                LOG_WARNING("Framebuffer attachment is neither a depth stencil or a render target");
                continue;
            }

            renderTargets.push_back(pTextureDX->getRTV());
        } else {
            if (pDSV) {
                LOG_WARNING("Multiple depth stencils in framebuffer info");
                continue;
            }

            pDSV = pTextureDX->getDSV();
            depthStencilIdx = textureIdx;
        }
    }

    return DBG_NEW FramebufferDX11(renderTargets, pDSV, depthStencilIdx);
}

FramebufferDX11::FramebufferDX11(const std::vector<ID3D11RenderTargetView*>& renderTargets, ID3D11DepthStencilView* pDepthStencil, size_t depthStencilIdx)
    :m_RenderTargets(renderTargets),
    m_pDepthStencil(pDepthStencil),
    m_DepthStencilIdx(depthStencilIdx)
{}
