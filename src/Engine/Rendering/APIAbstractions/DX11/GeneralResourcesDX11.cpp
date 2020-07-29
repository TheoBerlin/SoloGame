#include "GeneralResourcesDX11.hpp"

DXGI_FORMAT convertFormatToDX(RESOURCE_FORMAT textureFormat)
{
    switch (textureFormat) {
        case RESOURCE_FORMAT::R32G32B32A32_FLOAT:
            return DXGI_FORMAT_R32G32B32A32_FLOAT;
        case RESOURCE_FORMAT::R32G32B32_FLOAT:
            return DXGI_FORMAT_R32G32B32_FLOAT;
        case RESOURCE_FORMAT::R32G32_FLOAT:
            return DXGI_FORMAT_R32G32_FLOAT;
        case RESOURCE_FORMAT::B8G8R8A8_UNORM:
            return DXGI_FORMAT_B8G8R8A8_UNORM;
        case RESOURCE_FORMAT::B8G8R8A8_SRGB:
            return DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
        case RESOURCE_FORMAT::R8G8B8A8_UNORM:
            return DXGI_FORMAT_R8G8B8A8_UNORM;
        case RESOURCE_FORMAT::R8G8B8A8_SRGB:
            return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
        case RESOURCE_FORMAT::D32_FLOAT:
            return DXGI_FORMAT_D32_FLOAT;
        default:
            LOG_ERROR("Unknown resource format");
            return DXGI_FORMAT_UNKNOWN;
    }
}

RESOURCE_FORMAT convertFormatFromDX(DXGI_FORMAT format)
{
    switch (format) {
        case DXGI_FORMAT_R32G32B32A32_FLOAT:
            return RESOURCE_FORMAT::R32G32B32A32_FLOAT;
        case DXGI_FORMAT_R32G32B32_FLOAT:
            return RESOURCE_FORMAT::R32G32B32_FLOAT;
        case DXGI_FORMAT_R32G32_FLOAT:
            return RESOURCE_FORMAT::R32G32_FLOAT;
        case DXGI_FORMAT_B8G8R8A8_UNORM:
            return RESOURCE_FORMAT::B8G8R8A8_UNORM;
        case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
            return RESOURCE_FORMAT::B8G8R8A8_SRGB;
        case DXGI_FORMAT_R8G8B8A8_UNORM:
            return RESOURCE_FORMAT::R8G8B8A8_UNORM;
        case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
            return RESOURCE_FORMAT::R8G8B8A8_SRGB;
        case DXGI_FORMAT_D32_FLOAT:
            return RESOURCE_FORMAT::D32_FLOAT;

        default:
            LOG_ERRORF("Unknown resource format: %d", (int)format);
            return RESOURCE_FORMAT::R32G32B32A32_FLOAT;
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
            LOG_WARNINGF("Erroneous comparison function: %d", (int)comparisonFunc);
            return D3D11_COMPARISON_ALWAYS;
    }
}

D3D11_RECT convertRectangle(const Rectangle2D& rectangle)
{
    D3D11_RECT rectDX = {};
    rectDX.left     = (LONG)rectangle.Offset.x;
    rectDX.top      = (LONG)rectangle.Offset.y;
    rectDX.right    = LONG(rectangle.Offset.x + rectangle.Extent.x);
    rectDX.bottom   = LONG(rectangle.Offset.y + rectangle.Extent.y);

    return rectDX;
}
