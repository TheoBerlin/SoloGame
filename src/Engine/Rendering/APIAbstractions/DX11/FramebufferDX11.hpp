#pragma once

#include <Engine/Rendering/APIAbstractions/Framebuffer.hpp>

#define NOMINMAX
#include <d3d11.h>
#include <vector>

class FramebufferDX11 : public IFramebuffer
{
public:
    static FramebufferDX11* create(const FramebufferInfo& framebufferInfo);

public:
    FramebufferDX11(const std::vector<ID3D11RenderTargetView*>& renderTargets, ID3D11DepthStencilView* pDepthStencil, size_t depthStencilIdx);
    ~FramebufferDX11() = default;

    inline std::vector<ID3D11RenderTargetView*>& getRenderTargets() { return m_RenderTargets; }
    inline ID3D11DepthStencilView* getDepthStencil()                { return m_pDepthStencil; }
    inline size_t getDepthStencilIdx() const                        { return m_DepthStencilIdx; }

private:
    std::vector<ID3D11RenderTargetView*> m_RenderTargets;
    ID3D11DepthStencilView* m_pDepthStencil;
    // Vulkan framebuffers specify all textures (including the depth/stencil texture) in a single array.
    // But when clearing a depth-stencil texture in DX11, we need to know which of the textures is the depth-stencil texture. Hence the index.
    size_t m_DepthStencilIdx;
};
