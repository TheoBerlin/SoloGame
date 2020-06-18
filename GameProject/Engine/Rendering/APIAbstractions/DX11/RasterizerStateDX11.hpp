#pragma once

#include <Engine/Rendering/APIAbstractions/RasterizerState.hpp>

#define NOMINMAX
#include <d3d11.h>

struct RasterizerStateDX11 {
    ID3D11RasterizerState* pRasterizerState;
};

bool createRasterizerState(RasterizerStateDX11& rasterizerState, const RasterizerStateInfo& rasterizerInfo, ID3D11Device* pDevice);
