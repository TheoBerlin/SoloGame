#include "GeneralResourcesDX11.hpp"

DXGI_FORMAT convertFormat(RESOURCE_FORMAT textureFormat)
{
    switch (textureFormat) {
        case RESOURCE_FORMAT::R32G32B32A32_FLOAT:
            return DXGI_FORMAT_R32G32B32A32_FLOAT;
        case RESOURCE_FORMAT::R32G32B32_FLOAT:
            return DXGI_FORMAT_R32G32B32_FLOAT;
        case RESOURCE_FORMAT::R32G32_FLOAT:
            return DXGI_FORMAT_R32G32_FLOAT;
        case RESOURCE_FORMAT::R8G8B8A8_UNORM:
            return DXGI_FORMAT_R8G8B8A8_UNORM;
        case RESOURCE_FORMAT::D32_FLOAT:
            return DXGI_FORMAT_D32_FLOAT;
        default:
            LOG_ERROR("Unknown resource format");
            return DXGI_FORMAT_UNKNOWN;
    }
}

D3D11_COMPARISON_FUNC convertComparisonFunc(COMPARISON_FUNC comparisonFunc)
{
    switch(comparisonFunc) {
        case COMPARISON_FUNC::NEVER:
            return D3D11_COMPARISON_NEVER;
        case COMPARISON_FUNC::LESS:
            return D3D11_COMPARISON_LESS;
        case COMPARISON_FUNC::LESS_OR_EQUAL:
            return D3D11_COMPARISON_LESS_EQUAL;
        case COMPARISON_FUNC::EQUAL:
            return D3D11_COMPARISON_EQUAL;
        case COMPARISON_FUNC::EQUAL_OR_GREATER:
            return D3D11_COMPARISON_GREATER_EQUAL;
        case COMPARISON_FUNC::GREATER:
            return D3D11_COMPARISON_GREATER;
        case COMPARISON_FUNC::ALWAYS:
            return D3D11_COMPARISON_ALWAYS;
        default:
            LOG_WARNING("Erroneous comparison function: %d", (int)comparisonFunc);
            return D3D11_COMPARISON_ALWAYS;
    }
}