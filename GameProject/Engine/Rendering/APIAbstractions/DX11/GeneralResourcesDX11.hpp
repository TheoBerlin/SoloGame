#pragma once

#include <Engine/Rendering/APIAbstractions/GeneralResources.hpp>

#define NOMINMAX
#include <d3d11.h>

DXGI_FORMAT convertFormat(RESOURCE_FORMAT textureFormat);

D3D11_COMPARISON_FUNC convertComparisonFunc(COMPARISON_FUNC comparisonFunc);
