#pragma once

#include <Engine/Rendering/APIAbstractions/InputLayout.hpp>

#define NOMINMAX
#include <d3d11.h>

class InputLayoutDX11 : public InputLayout
{
public:
    static InputLayoutDX11* create(const InputLayoutInfo* pInputLayoutInfo, ID3DBlob* pShaderCode, ID3D11Device* pDevice);

public:
    InputLayoutDX11(ID3D11InputLayout* pInputLayout, uint32_t vertexSize);
    ~InputLayoutDX11();

    inline ID3D11InputLayout* getInputLayout() { return m_pInputLayout; }

private:
    ID3D11InputLayout* m_pInputLayout;
};
