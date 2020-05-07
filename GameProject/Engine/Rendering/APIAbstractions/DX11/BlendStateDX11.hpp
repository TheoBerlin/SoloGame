#pragma once

#include <Engine/Rendering/APIAbstractions/BlendState.hpp>

#define NOMINMAX
#include <d3d11.h>

class BlendStateDX11 : public BlendState
{
public:
    static BlendStateDX11* create(const BlendStateInfo& blendStateInfo, ID3D11Device* pDevice);

public:
    BlendStateDX11(ID3D11BlendState* pBlendState, const float pBlendConstants[4]);
    ~BlendStateDX11();

    ID3D11BlendState* getBlendState() { return m_pBlendState; }

private:
    static D3D11_BLEND convertBlendFactor(BLEND_FACTOR blendFactor);
    static D3D11_BLEND_OP convertBlendOp(BLEND_OP blendOp);

private:
    ID3D11BlendState* m_pBlendState;
};
