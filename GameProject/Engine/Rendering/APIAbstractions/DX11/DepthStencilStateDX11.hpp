#pragma once

#include <Engine/Rendering/APIAbstractions/DepthStencilState.hpp>

#define NOMINMAX
#include <d3d11.h>

struct DepthStencilStateDX11 {
    ID3D11DepthStencilState* pDepthStencilState;
    UINT StencilReference;
};

bool createDepthStencilState(DepthStencilStateDX11& depthStencilState, const DepthStencilInfo& depthStencilInfo, ID3D11Device* pDevice);

D3D11_DEPTH_STENCILOP_DESC convertStencilOpInfo(const StencilOpInfo& stencilOpInfo);
D3D11_STENCIL_OP convertStencilOp(STENCIL_OP stencilOp);
