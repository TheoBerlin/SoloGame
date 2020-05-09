#pragma once

#include <Engine/Rendering/APIAbstractions/DepthStencilState.hpp>

#define NOMINMAX
#include <d3d11.h>

class DepthStencilStateDX11 : public IDepthStencilState
{
public:
    static DepthStencilStateDX11* create(const DepthStencilInfo& depthStencilInfo, ID3D11Device* pDevice);

public:
    DepthStencilStateDX11(ID3D11DepthStencilState* pDepthStencilState, UINT stencilReference);
    ~DepthStencilStateDX11();

    inline ID3D11DepthStencilState* getDepthStencilState() { return m_pDepthStencilState; }
    inline UINT getStencilReference() const { return m_StencilReference; }

private:
    static D3D11_DEPTH_STENCILOP_DESC convertStencilOpInfo(const StencilOpInfo& stencilOpInfo);
    static D3D11_STENCIL_OP convertStencilOp(STENCIL_OP stencilOp);

private:
    ID3D11DepthStencilState* m_pDepthStencilState;

    UINT m_StencilReference;
};
