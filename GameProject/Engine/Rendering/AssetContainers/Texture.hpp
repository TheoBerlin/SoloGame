#pragma once

#include <Engine/Rendering/AssetContainers/AssetResources.hpp>

struct ID3D11ShaderResourceView;

class Texture
{
public:
    Texture(ID3D11ShaderResourceView* pSRV = nullptr);
    ~Texture();

    ID3D11ShaderResourceView* getSRV() const    { return m_pSRV; }
    void setSRV(ID3D11ShaderResourceView* pSRV) { m_pSRV = pSRV; }

private:
    ID3D11ShaderResourceView* m_pSRV;
};
