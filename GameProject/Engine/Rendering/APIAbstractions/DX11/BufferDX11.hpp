#pragma once

#include <Engine/Rendering/APIAbstractions/IBuffer.hpp>
#include <Engine/Rendering/APIAbstractions/Shader.hpp>

class DeviceDX11;
struct ID3D11Buffer;
struct ID3D11DeviceContext;

class BufferDX11
{
public:
    BufferDX11(DeviceDX11* pDevice, const BufferInfo& bufferInfo);
    ~BufferDX11();

    void bind(SHADER_TYPE shaderStageFlags, int slot, ID3D11DeviceContext* pContext);

    ID3D11Buffer* getBuffer() { return m_pBuffer; }

private:
    ID3D11Buffer* m_pBuffer;
};
