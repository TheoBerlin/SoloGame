#include "BlendStateDX11.hpp"

#include <Engine/Utils/DirectXUtils.hpp>
#include <Engine/Utils/Logger.hpp>

#include <algorithm>
#include <vector>

BlendStateDX11* BlendStateDX11::create(const BlendStateInfo& blendStateInfo, ID3D11Device* pDevice)
{
    if (blendStateInfo.BlendInfosCount > 8) {
        LOG_ERROR("DirectX 11 does not support more than 8 simultaneous render targets, attempted nr: %d", blendStateInfo.BlendInfosCount);
    }

    size_t blendInfosCount = (size_t)std::min(blendStateInfo.BlendInfosCount, 8u);

    D3D11_BLEND_DESC blendDesc = {};
    blendDesc.AlphaToCoverageEnable = FALSE;
    blendDesc.IndependentBlendEnable = (BOOL)blendStateInfo.IndependentBlendEnabled;

    // Convert blend states
    for (size_t blendDescIdx = 0; blendDescIdx < blendInfosCount; blendDescIdx++) {
        BlendRenderTargetInfo& rtvBlendInfo = blendStateInfo.pRenderTargetBlendInfos[blendDescIdx];

        D3D11_RENDER_TARGET_BLEND_DESC& rtvBlendDesc = blendDesc.RenderTarget[blendDescIdx];
        rtvBlendDesc = {};
        rtvBlendDesc.BlendEnable            = (BOOL)rtvBlendInfo.BlendEnabled;
        rtvBlendDesc.SrcBlend               = convertBlendFactor(rtvBlendInfo.SrcColorBlendFactor);
        rtvBlendDesc.DestBlend              = convertBlendFactor(rtvBlendInfo.DstColorBlendFactor);
        rtvBlendDesc.BlendOp                = convertBlendOp(rtvBlendInfo.ColorBlendOp);
        rtvBlendDesc.SrcBlendAlpha          = convertBlendFactor(rtvBlendInfo.SrcAlphaBlendFactor);
        rtvBlendDesc.DestBlendAlpha         = convertBlendFactor(rtvBlendInfo.DstAlphaBlendFactor);
        rtvBlendDesc.BlendOpAlpha           = convertBlendOp(rtvBlendInfo.AlphaBlendOp);
        rtvBlendDesc.RenderTargetWriteMask  = (UINT8)rtvBlendInfo.ColorWriteMask;
    }

    ID3D11BlendState* pBlendState = nullptr;
    HRESULT hr = pDevice->CreateBlendState(&blendDesc, &pBlendState);
    if (FAILED(hr)) {
        LOG_ERROR("Failed to create blend state: %s", hresultToString(hr).c_str());
        SAFERELEASE(pBlendState)
        return nullptr;
    }

    return new BlendStateDX11(pBlendState, blendStateInfo.pBlendConstants);
}

BlendStateDX11::BlendStateDX11(ID3D11BlendState* pBlendState, const float pBlendConstants[4])
    :BlendState(pBlendConstants),
    m_pBlendState(pBlendState)
{}

BlendStateDX11::~BlendStateDX11()
{
    SAFERELEASE(m_pBlendState)
}

D3D11_BLEND BlendStateDX11::convertBlendFactor(BLEND_FACTOR blendFactor)
{
    switch (blendFactor) {
        case BLEND_FACTOR::ZERO:
            return D3D11_BLEND_ZERO;
        case BLEND_FACTOR::ONE:
            return D3D11_BLEND_ONE;
        case BLEND_FACTOR::SRC_COLOR:
            return D3D11_BLEND_SRC_COLOR;
        case BLEND_FACTOR::ONE_MINUS_SRC_COLOR:
            return D3D11_BLEND_INV_SRC_COLOR;
        case BLEND_FACTOR::DST_COLOR:
            return D3D11_BLEND_DEST_COLOR;
        case BLEND_FACTOR::ONE_MINUS_DST_COLOR:
            return D3D11_BLEND_INV_DEST_COLOR;
        case BLEND_FACTOR::SRC_ALPHA:
            return D3D11_BLEND_SRC_ALPHA;
        case BLEND_FACTOR::ONE_MINUS_SRC_ALPHA:
            return D3D11_BLEND_INV_SRC_ALPHA;
        case BLEND_FACTOR::DST_ALPHA:
            return D3D11_BLEND_DEST_ALPHA;
        case BLEND_FACTOR::ONE_MINUS_DST_ALPHA:
            return D3D11_BLEND_INV_DEST_ALPHA;
        case BLEND_FACTOR::CONSTANT_COLOR:
        case BLEND_FACTOR::CONSTANT_ALPHA:
            return D3D11_BLEND_BLEND_FACTOR;
        case BLEND_FACTOR::ONE_MINUS_CONSTANT_COLOR:
        case BLEND_FACTOR::ONE_MINUS_CONSTANT_ALPHA:
            return D3D11_BLEND_INV_BLEND_FACTOR;
        case BLEND_FACTOR::SRC_ALPHA_SATURATE:
            return D3D11_BLEND_SRC_ALPHA_SAT;
        case BLEND_FACTOR::SRC1_COLOR:
            return D3D11_BLEND_SRC1_COLOR;
        case BLEND_FACTOR::ONE_MINUS_SRC1_COLOR:
            return D3D11_BLEND_INV_SRC1_COLOR;
        case BLEND_FACTOR::SRC1_ALPHA:
            return D3D11_BLEND_SRC1_ALPHA;
        case BLEND_FACTOR::ONE_MINUS_SRC1_ALPHA:
            return D3D11_BLEND_INV_SRC1_ALPHA;
        default:
            LOG_WARNING("Erroneous blend factor: %d", (int)blendFactor);
            return D3D11_BLEND_ONE;
    }
}

D3D11_BLEND_OP BlendStateDX11::convertBlendOp(BLEND_OP blendOp)
{
    switch (blendOp) {
        case BLEND_OP::ADD:
            return D3D11_BLEND_OP_ADD;
        case BLEND_OP::SUBTRACT:
            return D3D11_BLEND_OP_SUBTRACT;
        case BLEND_OP::REVERSE_SUBTRACT:
            return D3D11_BLEND_OP_REV_SUBTRACT;
        case BLEND_OP::MIN:
            return D3D11_BLEND_OP_MIN;
        case BLEND_OP::MAX:
            return D3D11_BLEND_OP_MAX;
        default:
            LOG_WARNING("Erroneous blend op: %d", (int)blendOp);
            return D3D11_BLEND_OP_ADD;
    }
}