#include "BufferDX11.hpp"

#include <Engine/Rendering/APIAbstractions/DX11/DeviceDX11.hpp>
#include <Engine/Utils/DirectXUtils.hpp>
#include <Engine/Utils/Logger.hpp>

BufferDX11::BufferDX11(ID3D11Device* pDevice, const BufferInfo& bufferInfo)
    :m_pBuffer(nullptr)
{
    // The size of DirectX 11 constant buffers needs to be a multiple of 16
    UINT byteSize = (UINT)bufferInfo.ByteSize;
    if (bufferInfo.Usage == BUFFER_USAGE::UNIFORM_BUFFER) {
        UINT remainder = byteSize % 16u;
        if (remainder != 0u) {
            byteSize += 16u - remainder;
        }
    }

    D3D11_BUFFER_DESC bufferDesc = {};
    bufferDesc.ByteWidth = byteSize;

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
            return;
    }

    bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ * HAS_FLAG(bufferInfo.CPUAccess, BUFFER_DATA_ACCESS::READ)
                            | D3D11_CPU_ACCESS_WRITE * HAS_FLAG(bufferInfo.CPUAccess, BUFFER_DATA_ACCESS::WRITE);
    bufferDesc.MiscFlags = 0;
    bufferDesc.StructureByteStride = 0;

    D3D11_SUBRESOURCE_DATA bufferData;
    bufferData.pSysMem = bufferInfo.pData;
    bufferData.SysMemPitch = 0;
    bufferData.SysMemSlicePitch = 0;

    HRESULT hr = pDevice->CreateBuffer(&bufferDesc, bufferInfo.pData ? &bufferData : nullptr, &m_pBuffer);
    if (FAILED(hr)) {
        LOG_WARNING("Failed to create buffer: %s", hresultToString(hr).c_str());
    }
}

BufferDX11::~BufferDX11()
{
    SAFERELEASE(m_pBuffer)
}
