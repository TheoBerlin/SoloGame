#include "Texture.hpp"

#include <Engine/Utils/DirectXUtils.hpp>

Texture::Texture(ID3D11ShaderResourceView* pSRV)
    :m_pSRV(pSRV)
{}

Texture::~Texture()
{
    SAFERELEASE(m_pSRV)
}
