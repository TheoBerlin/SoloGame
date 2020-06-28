#pragma once

#include <Engine/Rendering/APIAbstractions/RasterizerState.hpp>

#define NOMINMAX
#include <d3d11.h>

bool createRasterizerState(ID3D11RasterizerState** ppRasterizerState, const RasterizerStateInfo& rasterizerInfo, ID3D11Device* pDevice);
