#pragma once

#include <Engine/Rendering/APIAbstractions/Framebuffer.hpp>
#include <Engine/Rendering/APIAbstractions/GeneralResources.hpp>

#define NOMINMAX
#include <d3d11.h>
#include <vector>

struct RenderTargetInfo {
    ID3D11RenderTargetView* pRTV;
    FORMAT_PRIMITIVE_TYPE FormatType;
};

class FramebufferDX11 : public Framebuffer
{
public:
    static FramebufferDX11* create(const FramebufferInfo& framebufferInfo);

public:
    FramebufferDX11(const std::vector<RenderTargetInfo>& renderTargets, ID3D11DepthStencilView* pDepthStencil, size_t depthStencilIdx, const glm::uvec2& dimensions);
    ~FramebufferDX11() = default;

    inline std::vector<RenderTargetInfo>& getRenderTargets()    { return m_RenderTargets; }
    inline ID3D11DepthStencilView* getDepthStencil()            { return m_pDepthStencil; }
    inline size_t getDepthStencilIdx() const                    { return m_DepthStencilIdx; }

private:
    std::vector<RenderTargetInfo> m_RenderTargets;
    ID3D11DepthStencilView* m_pDepthStencil;
    // Vulkan framebuffers specify all textures (including the depth/stencil texture) in a single array.
    // But when clearing a depth-stencil texture in DX11, we need to know which of the textures is the depth-stencil texture. Hence the index.
    size_t m_DepthStencilIdx;
};
