#include "BufferDX11.hpp"

#include <Engine/Rendering/APIAbstractions/DX11/DeviceDX11.hpp>
#include <Engine/Utils/DirectXUtils.hpp>
#include <Engine/Utils/Logger.hpp>

BufferDX11::BufferDX11(DeviceDX11* pDevice, const BufferInfo& bufferInfo)
    :m_pBuffer(nullptr)
{
    D3D11_BUFFER_DESC bufferDesc;
    ZeroMemory(&bufferDesc, sizeof(D3D11_BUFFER_DESC));
    bufferDesc.ByteWidth = (UINT)bufferInfo.ByteSize;

    // Figure out which usage flag to use
    if (bufferInfo.Usage == BUFFER_USAGE::STAGING_BUFFER) {
        bufferDesc.Usage = D3D11_USAGE_STAGING;
    } else if (!HAS_FLAG(bufferInfo.CPUAccess | bufferInfo.GPUAccess, BUFFER_DATA_ACCESS::WRITE)) {
        bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
    } else if (HAS_FLAG(bufferInfo.CPUAccess, BUFFER_DATA_ACCESS::WRITE)) {
        bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    } else {
        bufferDesc.Usage = D3D11_USAGE_DEFAULT;
    }

    // Decide BindFlag
    switch (bufferInfo.Usage) {
        case BUFFER_USAGE::VERTEX_BUFFER:
            bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
            break;
        case BUFFER_USAGE::INDEX_BUFFER:
            bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
            break;
        case BUFFER_USAGE::UNIFORM_BUFFER:
        case BUFFER_USAGE::STAGING_BUFFER:
            bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
            break;
        default:
            LOG_ERROR("Invalid buffer usage flag: %d", bufferInfo.Usage);
            break;
    }

    bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ * HAS_FLAG(bufferInfo.CPUAccess, BUFFER_DATA_ACCESS::READ)
                            | D3D11_CPU_ACCESS_WRITE * HAS_FLAG(bufferInfo.CPUAccess, BUFFER_DATA_ACCESS::READ);
    bufferDesc.MiscFlags = 0;
    bufferDesc.StructureByteStride = 0;

    D3D11_SUBRESOURCE_DATA bufferData;
    bufferData.pSysMem = bufferInfo.pData;
    bufferData.SysMemPitch = 0;
    bufferData.SysMemSlicePitch = 0;

    HRESULT hr = pDevice->getDevice()->CreateBuffer(&bufferDesc, bufferInfo.pData ? &bufferData : nullptr, &m_pBuffer);
    if (FAILED(hr)) {
        LOG_WARNING("Failed to create buffer: %s", hresultToString(hr).c_str());
        return;
    }

    return;
}

void BufferDX11::bind(SHADER_TYPE shaderStageFlags, int slot, ID3D11DeviceContext* pContext)
{
    if (HAS_FLAG(shaderStageFlags, SHADER_TYPE::VERTEX_SHADER)) {
        pContext->VSSetConstantBuffers((UINT)slot, 1, &m_pBuffer);
    }

    if (HAS_FLAG(shaderStageFlags, SHADER_TYPE::HULL_SHADER)) {
        pContext->HSSetConstantBuffers((UINT)slot, 1, &m_pBuffer);
    }

    if (HAS_FLAG(shaderStageFlags, SHADER_TYPE::DOMAIN_SHADER)) {
        pContext->DSSetConstantBuffers((UINT)slot, 1, &m_pBuffer);
    }

    if (HAS_FLAG(shaderStageFlags, SHADER_TYPE::GEOMETRY_SHADER)) {
        pContext->GSSetConstantBuffers((UINT)slot, 1, &m_pBuffer);
    }

    if (HAS_FLAG(shaderStageFlags, SHADER_TYPE::FRAGMENT_SHADER)) {
        pContext->PSSetConstantBuffers((UINT)slot, 1, &m_pBuffer);
    }
}
