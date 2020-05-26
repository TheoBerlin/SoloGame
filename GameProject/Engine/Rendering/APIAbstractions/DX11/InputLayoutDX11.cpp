#include "InputLayoutDX11.hpp"

#include <Engine/Rendering/APIAbstractions/DX11/GeneralResourcesDX11.hpp>
#include <Engine/Utils/Debug.hpp>
#include <Engine/Utils/DirectXUtils.hpp>
#include <Engine/Utils/Logger.hpp>

D3D11_PRIMITIVE_TOPOLOGY convertPrimitiveTopology(PRIMITIVE_TOPOLOGY primitiveTopology)
{
    switch (primitiveTopology) {
        case PRIMITIVE_TOPOLOGY::POINT_LIST:
            return D3D11_PRIMITIVE_TOPOLOGY_POINTLIST;
        case PRIMITIVE_TOPOLOGY::LINE_LIST:
            return D3D11_PRIMITIVE_TOPOLOGY_LINELIST;
        case PRIMITIVE_TOPOLOGY::LINE_STRIP:
            return D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP;
        case PRIMITIVE_TOPOLOGY::TRIANGLE_LIST:
            return D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
        case PRIMITIVE_TOPOLOGY::TRIANGLE_STRIP:
            return D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
        case PRIMITIVE_TOPOLOGY::LINE_LIST_WITH_ADJACENCY:
            return D3D11_PRIMITIVE_TOPOLOGY_LINELIST_ADJ;
        case PRIMITIVE_TOPOLOGY::LINE_STRIP_WITH_ADJACENCY:
            return D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP_ADJ;
        case PRIMITIVE_TOPOLOGY::TRIANGLE_LIST_WITH_ADJACENCY:
            return D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST_ADJ;
        case PRIMITIVE_TOPOLOGY::TRIANGLE_STRIP_WITH_ADJACENCY:
            return D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP_ADJ;
        default:
            LOG_ERROR("Erroneous primitive topology: %d", (int)primitiveTopology);
            return D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED;
    }
}

InputLayoutDX11* InputLayoutDX11::create(const InputLayoutInfo* pInputLayoutInfo, ID3DBlob* pShaderCode, ID3D11Device* pDevice)
{
    std::vector<D3D11_INPUT_ELEMENT_DESC> attributeDescs(pInputLayoutInfo->VertexInputAttributes.size());
    UINT attributeOffset = 0u;

    for (size_t attributeIdx = 0; attributeIdx < attributeDescs.size(); attributeIdx++) {
        D3D11_INPUT_ELEMENT_DESC& attributeDesc     = attributeDescs[attributeIdx];
        const InputVertexAttribute& attributeInfo   = pInputLayoutInfo->VertexInputAttributes[attributeIdx];

        attributeDesc.SemanticName          = (LPCSTR)attributeInfo.SemanticName.c_str();
        attributeDesc.SemanticIndex         = 0u;
        attributeDesc.Format                = convertFormatToDX(attributeInfo.Format);
        attributeDesc.InputSlot             = (UINT)pInputLayoutInfo->Binding;
        attributeDesc.AlignedByteOffset     = attributeOffset;
        attributeDesc.InputSlotClass        = attributeInfo.InputRate == VERTEX_INPUT_RATE::PER_VERTEX ? D3D11_INPUT_PER_VERTEX_DATA : D3D11_INPUT_PER_INSTANCE_DATA;
        attributeDesc.InstanceDataStepRate  = 0u;

        attributeOffset += (UINT)getFormatSize(attributeInfo.Format);
    }

    ID3D11InputLayout* pInputLayout = nullptr;
    HRESULT hr = pDevice->CreateInputLayout(attributeDescs.data(), (UINT)attributeDescs.size(), pShaderCode->GetBufferPointer(), pShaderCode->GetBufferSize(), &pInputLayout);
    if (FAILED(hr)) {
        LOG_ERROR("Failed to create input layout: %s", hresultToString(hr).c_str());
        SAFERELEASE(pInputLayout)
        return nullptr;
    }

    return DBG_NEW InputLayoutDX11(pInputLayout, (uint32_t)attributeOffset);
}

InputLayoutDX11::InputLayoutDX11(ID3D11InputLayout* pInputLayout, uint32_t vertexSize)
    :InputLayout(vertexSize),
    m_pInputLayout(pInputLayout)
{}

InputLayoutDX11::~InputLayoutDX11()
{
    SAFERELEASE(m_pInputLayout)
}
