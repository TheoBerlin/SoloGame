#pragma once

#include <Engine/Rendering/APIAbstractions/InputLayout.hpp>

#define NOMINMAX
#include <d3d11.h>

struct InputLayoutDX11 {
    ID3D11InputLayout* pInputLayout;
    UINT VertexSize;
};

bool createInputLayout(InputLayoutDX11& inputLayout, const InputLayoutInfo* pInputLayoutInfo, ID3DBlob* pShaderCode, ID3D11Device* pDevice);

D3D11_PRIMITIVE_TOPOLOGY convertPrimitiveTopology(PRIMITIVE_TOPOLOGY primitiveTopology);
