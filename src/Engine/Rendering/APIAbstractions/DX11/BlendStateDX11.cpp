#include "BlendStateDX11.hpp"

#include <Engine/Utils/Debug.hpp>
#include <Engine/Utils/DirectXUtils.hpp>
#include <Engine/Utils/Logger.hpp>

#include <algorithm>
#include <vector>

bool createBlendState(BlendStateDX11& blendState, const BlendStateInfo& blendStateInfo, ID3D11Device* pDevice)
{
    if (blendStateInfo.RenderTargetBlendInfos.size() > 8) {
        LOG_ERRORF("DirectX 11 does not support more than 8 simultaneous render targets, attempted nr: %d", blendStateInfo.RenderTargetBlendInfos.size());
    }

    blendState = {};
    std::memcpy(blendState.pBlendConstants, blendStateInfo.pBlendConstants, sizeof(FLOAT) * 4u);

    size_t blendInfosCount = std::min(blendStateInfo.RenderTargetBlendInfos.size(), size_t(8u));

    D3D11_BLEND_DESC blendDesc = {};
    blendDesc.AlphaToCoverageEnable = FALSE;
    blendDesc.IndependentBlendEnable = (BOOL)blendStateInfo.IndependentBlendEnabled;

    // Convert blend states
    for (size_t blendDescIdx = 0; blendDescIdx < blendInfosCount; blendDescIdx++) {
        const BlendRenderTargetInfo& rtvBlendInfo = blendStateInfo.RenderTargetBlendInfos[blendDescIdx];

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

    HRESULT hr = pDevice->CreateBlendState(&blendDesc, &blendState.pBlendState);
    if (FAILED(hr)) {
        LOG_ERRORF("Failed to create blend state: %s", hresultToString(hr).c_str());
        SAFERELEASE(blendState.pBlendState)
        return false;
    }

    return true;
}

D3D11_BLEND convertBlendFactor(BLEND_FACTOR blendFactor)
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
            LOG_WARNINGF("Erroneous blend factor: %d", (int)blendFactor);
            return D3D11_BLEND_ONE;
    }
}

D3D11_BLEND_OP convertBlendOp(BLEND_OP blendOp)
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
            LOG_WARNINGF("Erroneous blend op: %d", (int)blendOp);
            return D3D11_BLEND_OP_ADD;
    }
}
