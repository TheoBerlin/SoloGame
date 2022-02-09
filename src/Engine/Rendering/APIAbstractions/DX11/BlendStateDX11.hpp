#pragma once

#include <Engine/Rendering/APIAbstractions/BlendState.hpp>

#include <d3d11.h>

struct BlendStateDX11 {
    ID3D11BlendState* pBlendState;
    float pBlendConstants[4];
};

bool createBlendState(BlendStateDX11& blendState, const BlendStateInfo& blendStateInfo, ID3D11Device* pDevice);

D3D11_BLEND convertBlendFactor(BLEND_FACTOR blendFactor);
D3D11_BLEND_OP convertBlendOp(BLEND_OP blendOp);
