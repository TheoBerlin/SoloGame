#include "RasterizerStateDX11.hpp"

#include <Engine/Utils/Debug.hpp>
#include <Engine/Utils/DirectXUtils.hpp>
#include <Engine/Utils/Logger.hpp>

bool createRasterizerState(ID3D11RasterizerState** ppRasterizerState, const RasterizerStateInfo& rasterizerInfo, ID3D11Device* pDevice)
{
    D3D11_RASTERIZER_DESC rsDesc = {};
    rsDesc.FillMode = rasterizerInfo.PolygonMode == POLYGON_MODE::FILL ? D3D11_FILL_SOLID : D3D11_FILL_WIREFRAME;

    switch (rasterizerInfo.CullMode) {
        case CULL_MODE::NONE:
            rsDesc.CullMode = D3D11_CULL_NONE;
            break;
        case CULL_MODE::FRONT:
            rsDesc.CullMode = D3D11_CULL_FRONT;
            break;
        case CULL_MODE::BACK:
            rsDesc.CullMode = D3D11_CULL_BACK;
            break;
        default:
            LOG_ERRORF("Erroneous cull mode flag: %d", (int)rsDesc.CullMode);
            return false;
    }

    rsDesc.FrontCounterClockwise    = BOOL(rasterizerInfo.FrontFaceOrientation == FRONT_FACE_ORIENTATION::COUNTER_CLOCKWISE);
    rsDesc.DepthClipEnable          = (BOOL)rasterizerInfo.DepthBiasEnable;
    rsDesc.DepthBias                = (INT)rasterizerInfo.DepthBiasConstantFactor;
    rsDesc.SlopeScaledDepthBias     = (FLOAT)rasterizerInfo.DepthBiasSlopeFactor;
    rsDesc.DepthBiasClamp           = (FLOAT)rasterizerInfo.DepthBiasClamp;
    rsDesc.ScissorEnable            = FALSE;
    rsDesc.AntialiasedLineEnable    = false;
    rsDesc.MultisampleEnable        = false;

    HRESULT hr = pDevice->CreateRasterizerState(&rsDesc, ppRasterizerState);
    if (FAILED(hr)) {
        LOG_ERROR("Failed to create rasterizer state");
        SAFERELEASE(*ppRasterizerState)
        return false;
    }

    return true;
}
