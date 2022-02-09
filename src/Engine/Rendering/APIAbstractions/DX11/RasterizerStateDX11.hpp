#pragma once

#include <Engine/Rendering/APIAbstractions/RasterizerState.hpp>

#include <d3d11.h>

bool createRasterizerState(ID3D11RasterizerState** ppRasterizerState, const RasterizerStateInfo& rasterizerInfo, ID3D11Device* pDevice);
