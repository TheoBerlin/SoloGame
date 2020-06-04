#pragma once

#include <Engine/Rendering/APIAbstractions/IBuffer.hpp>
#include <Engine/Rendering/APIAbstractions/Shader.hpp>

struct ID3D11Buffer;
struct ID3D11Device;

class BufferDX11 : public IBuffer
{
public:
    BufferDX11(ID3D11Device* pDevice, const BufferInfo& bufferInfo);
    ~BufferDX11();

    ID3D11Buffer* getBuffer() { return m_pBuffer; }

private:
    ID3D11Buffer* m_pBuffer;
};
