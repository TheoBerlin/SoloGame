#pragma once

#include <Engine/Rendering/APIAbstractions/IRasterizerState.hpp>

#define NOMINMAX
#include <d3d11.h>

class RasterizerStateDX11 : public IRasterizerState
{
public:
    static RasterizerStateDX11* create(const RasterizerStateInfo& rasterizerInfo, ID3D11Device* pDevice);

public:
    RasterizerStateDX11(ID3D11RasterizerState* pRasterizerState) : m_pRasterizerState(pRasterizerState) {};
    ~RasterizerStateDX11();

    ID3D11RasterizerState* getRasterizerState() { return m_pRasterizerState; }

private:
    ID3D11RasterizerState* m_pRasterizerState;
};
