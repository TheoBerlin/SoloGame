#pragma once

#include <Engine/Rendering/APIAbstractions/GeneralResources.hpp>

#include <d3d11.h>

DXGI_FORMAT convertFormatToDX(RESOURCE_FORMAT textureFormat);
RESOURCE_FORMAT convertFormatFromDX(DXGI_FORMAT format);

D3D11_COMPARISON_FUNC convertComparisonFunc(COMPARISON_FUNC comparisonFunc);

D3D11_RECT convertRectangle(const Rectangle2D& rectangle);
