#pragma once

#include <Engine/Rendering/APIAbstractions/RenderPass.hpp>

#define NOMINMAX
#include <d3d11.h>

class RenderPassDX11 : public IRenderPass
{
public:
    static RenderPassDX11* create(const RenderPassInfo& renderPassInfo);

public:
    RenderPassDX11(const std::vector<size_t>& clearIndices);
    ~RenderPassDX11() = default;

    void begin(const RenderPassBeginInfo& beginInfo, ID3D11DeviceContext* pContext);

private:
    // Indices to the render targets to clear when starting the render pass
    std::vector<size_t> m_ClearIndices;
};
