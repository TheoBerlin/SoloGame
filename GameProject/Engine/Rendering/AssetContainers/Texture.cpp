#include "Texture.hpp"

#include <Engine/Rendering/AssetContainers/AssetResources.hpp>
#include <Engine/Utils/DirectXUtils.hpp>

Texture::Texture()
    :m_RefCount(0),
    m_pSRV(nullptr)
{}

Texture::Texture(ID3D11ShaderResourceView* pSRV)
    :m_RefCount(0),
    m_pSRV(pSRV)
{}

Texture::~Texture()
{
    SAFERELEASE(m_pSRV)
}

void Texture::decreaseRefCount()
{
    m_RefCount -= 1;

    if (m_RefCount == 0 && m_pSRV != nullptr) {
        m_pSRV->Release();
        m_pSRV = nullptr;
    }
}
