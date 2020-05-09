#include "DepthStencilStateDX11.hpp"

#include <Engine/Rendering/APIAbstractions/DX11/GeneralResourcesDX11.hpp>
#include <Engine/Utils/DirectXUtils.hpp>
#include <Engine/Utils/Logger.hpp>

DepthStencilStateDX11* DepthStencilStateDX11::create(const DepthStencilInfo& depthStencilInfo, ID3D11Device* pDevice)
{
    D3D11_DEPTH_STENCIL_DESC depthStencilDesc = {};
    depthStencilDesc.DepthEnable        = (BOOL)depthStencilInfo.DepthTestEnabled;
    depthStencilDesc.DepthWriteMask     = depthStencilInfo.DepthWriteEnabled ? D3D11_DEPTH_WRITE_MASK_ALL : D3D11_DEPTH_WRITE_MASK_ZERO;
    depthStencilDesc.DepthFunc          = convertComparisonFunc(depthStencilInfo.DepthComparisonFunc);

    if (depthStencilInfo.StencilTestEnabled) {
        depthStencilDesc.StencilEnable      = TRUE;
        depthStencilDesc.FrontFace          = convertStencilOpInfo(depthStencilInfo.FrontFace);
        depthStencilDesc.BackFace           = convertStencilOpInfo(depthStencilInfo.BackFace);
        depthStencilDesc.StencilReadMask    = 0xFF;
        depthStencilDesc.StencilWriteMask   = 0xFF;
    }

    ID3D11DepthStencilState* pDepthStencilState = nullptr;
    HRESULT hr = pDevice->CreateDepthStencilState(&depthStencilDesc, &pDepthStencilState);
    if (FAILED(hr)) {
        LOG_ERROR("Failed to create depth stencil state: %s", hresultToString(hr).c_str());
        return nullptr;
    }

    return new DepthStencilStateDX11(pDepthStencilState, (UINT)depthStencilInfo.Reference);
}

DepthStencilStateDX11::DepthStencilStateDX11(ID3D11DepthStencilState* pDepthStencilState, UINT stencilReference)
    :m_pDepthStencilState(pDepthStencilState),
    m_StencilReference(stencilReference)
{}

DepthStencilStateDX11::~DepthStencilStateDX11()
{
    SAFERELEASE(m_pDepthStencilState)
}

D3D11_DEPTH_STENCILOP_DESC DepthStencilStateDX11::convertStencilOpInfo(const StencilOpInfo& stencilOpInfo)
{
    D3D11_DEPTH_STENCILOP_DESC stencilOpDesc = {};
    stencilOpDesc.StencilFailOp = convertStencilOp(stencilOpInfo.StencilFailOp);
    stencilOpDesc.StencilPassOp = convertStencilOp(stencilOpInfo.StencilPassOp);
    stencilOpDesc.StencilDepthFailOp = convertStencilOp(stencilOpInfo.DepthFailOp);
    stencilOpDesc.StencilFunc = convertComparisonFunc(stencilOpInfo.CompareFunc);

    return stencilOpDesc;
}

D3D11_STENCIL_OP DepthStencilStateDX11::convertStencilOp(STENCIL_OP stencilOp)
{
    switch (stencilOp) {
        case STENCIL_OP::KEEP:
            return D3D11_STENCIL_OP_KEEP;
        case STENCIL_OP::ZERO:
            return D3D11_STENCIL_OP_ZERO;
        case STENCIL_OP::REPLACE:
            return D3D11_STENCIL_OP_REPLACE;
        case STENCIL_OP::INCREMENT_AND_CLAMP:
            return D3D11_STENCIL_OP_INCR_SAT;
        case STENCIL_OP::DECREMENT_AND_CLAMP:
            return D3D11_STENCIL_OP_DECR_SAT;
        case STENCIL_OP::INVERT:
            return D3D11_STENCIL_OP_INVERT;
        case STENCIL_OP::INCREMENT_AND_WRAP:
            return D3D11_STENCIL_OP_INCR;
        case STENCIL_OP::DECREMENT_AND_WRAP:
            return D3D11_STENCIL_OP_DECR;
        default:
            LOG_WARNING("Erroneous stencil op: %d", (int)stencilOp);
            return D3D11_STENCIL_OP_KEEP;
    }
}
